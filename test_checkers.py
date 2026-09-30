"""Black-box tests for the staged C checkers exercise.

The checker does not inspect or prescribe the board's appearance. It only needs
an observable turn prompt so it can tell whether a move was accepted.

Console contract (also stated in the exercise PDF):
  * Run the binary without arguments; read one move per input line, e.g. b3 a4.
  * Print "White>" or "Black>" before every request, including a repeat after
    invalid input and each required jump in a capture chain.
  * Exit cleanly on EOF. Announce a winner using "White wins", "White won",
    or "Winner: White" (and the Black equivalent).

If the student chose other prompt wording, set CHECKERS_WHITE_PROMPT_RE and
CHECKERS_BLACK_PROMPT_RE to Python regular expressions matching their prompts.
The visual board format is unrestricted and is never parsed here.

Usage (from the directory containing the binary and this file):
    gcc -std=c11 -Wall -Wextra -Wpedantic checkers.c -o checkers
    python3 -m pip install -r requirements.txt
    python3 -m pytest -v test_checkers.py
    python3 -m pytest -v test_checkers.py -k stage3  # one stage

To use another binary path, set CHECKERS_BIN=/path/to/checkers.
"""

from __future__ import annotations

import os
from pathlib import Path
import re
import subprocess

import pytest


DEFAULT_PROMPTS = {
    "White": r"\bWhite\s*>",
    "Black": r"\bBlack\s*>",
}
WINNER_RE = re.compile(r"\b(?:white\s+(?:wins?|won)|winner\s*[:\-]?\s*white)\b", re.I)


def binary_path() -> Path:
    path = Path(os.environ.get("CHECKERS_BIN", "./checkers")).expanduser().resolve()
    if not path.is_file():
        pytest.fail(
            f"Binary not found: {path}\n"
            "Compile with: gcc -std=c11 -Wall -Wextra -Wpedantic checkers.c -o checkers\n"
            "Or set CHECKERS_BIN=/absolute/path/to/your/binary.",
            pytrace=False,
        )
    return path


def run_game(moves: list[str]) -> str:
    data = "\n".join(moves) + "\n"
    try:
        result = subprocess.run(
            [str(binary_path())], input=data, text=True,
            capture_output=True, timeout=10, check=False,
        )
    except subprocess.TimeoutExpired as exc:
        pytest.fail(
            f"The game did not exit within 10 seconds after EOF. "
            f"Input was: {moves!r}\nPartial output: {exc.stdout!r}",
            pytrace=False,
        )
    if result.returncode != 0:
        pytest.fail(
            f"The game exited with status {result.returncode}.\n"
            f"Input: {moves!r}\n"
            f"stdout:\n{result.stdout[-2500:]}\n"
            f"stderr:\n{result.stderr[-2500:]}",
            pytrace=False,
        )
    return result.stdout


def prompts_in(output: str) -> list[str]:
    patterns = {
        "White": os.environ.get("CHECKERS_WHITE_PROMPT_RE", DEFAULT_PROMPTS["White"]),
        "Black": os.environ.get("CHECKERS_BLACK_PROMPT_RE", DEFAULT_PROMPTS["Black"]),
    }
    events: list[tuple[int, str]] = []
    for player, pattern in patterns.items():
        try:
            events.extend((m.start(), player) for m in re.finditer(pattern, output, re.I))
        except re.error as exc:
            pytest.fail(f"Invalid {player} prompt regex {pattern!r}: {exc}", pytrace=False)
    events.sort()
    return [player for _, player in events]


def assert_turns(moves: list[str], before: list[str], after: str) -> None:
    """Append a deliberately invalid probe to observe whose turn follows."""
    assert len(moves) == len(before), "Test author error: one turn per input needed"
    output = run_game(moves + ["not-a-move"])
    actual = prompts_in(output)
    expected = before + [after]
    if len(actual) < len(expected) or actual[:len(expected)] != expected:
        pytest.fail(
            f"Wrong turn flow.\nMoves sent: {moves!r}\n"
            f"Expected prompts (before each move, then before probe): {expected}\n"
            f"Observed prompts: {actual}\n"
            "A repeated player means rejection or a required extra jump; "
            "a changed player means a completed turn.\n"
            f"Last output:\n{output[-3000:]}",
            pytrace=False,
        )


W, B = "White", "Black"


def test_stage1_white_starts() -> None:
    output = run_game(["not-a-move"])
    actual = prompts_in(output)
    assert actual and actual[0] == W, (
        f"Expected a White> prompt first; observed {actual}.\nOutput:\n{output[-1500:]}"
    )


def test_stage2_valid_turns_alternate() -> None:
    assert_turns(["b3 a4", "a6 b5"], [W, B], W)


def test_stage2_malformed_and_wrong_piece_do_not_use_a_turn() -> None:
    assert_turns(
        ["z9 a4", "hello", "a6 b5", "b3 a4", "b3 c4", "a6 b5"],
        [W, W, W, W, B, B], W,
    )


def test_stage3_illegal_geometry_and_occupied_square() -> None:
    assert_turns(
        ["b3 b4", "b3 c2", "b3 d5", "b3 c4", "a6 b5"],
        [W, W, W, W, B], W,
    )


def test_stage4_capture_is_accepted() -> None:
    assert_turns(["b3 a4", "c6 b5", "a4 c6"], [W, B, W], B)


def test_stage4_captured_piece_cannot_move_again() -> None:
    # White's g2 e4 jumps over Black's f3. If that Black piece remains,
    # f3 g2 would otherwise be a legal move on the next turn.
    assert_turns(
        ["h3 g4", "e6 d5", "f3 e4", "d5 f3", "g2 e4", "f3 g2", "a6 b5"],
        [W, B, W, B, W, B, B], W,
    )


def test_stage5_a_capture_blocks_a_quiet_move() -> None:
    assert_turns(
        ["b3 a4", "c6 b5", "f3 e4", "a4 c6"],
        [W, B, W, W], B,
    )


CHAIN = [
    "h3 g4", "e6 f5", "g4 e6", "f7 d5", "g2 h3",
    "c6 b5", "b3 a4", "e8 f7", "a4 c6", "c6 e8",
]


def test_stage5_multi_jump_keeps_the_same_player_and_piece() -> None:
    # Insert a different White piece's quiet move between the two jumps.
    # The last jump promotes the capturing piece on e8.
    moves = CHAIN[:-1] + ["f3 e4", CHAIN[-1]]
    assert_turns(moves, [W, B, W, B, W, B, W, B, W, W, W], B)


def test_stage6_a_new_king_can_move_backward() -> None:
    assert_turns(
        CHAIN + ["a6 b5", "e8 d7"],
        [W, B, W, B, W, B, W, B, W, W, B, W], B,
    )


WINNING_GAME = (
    "f3 g4|e6 d5|d3 c4|d7 e6|c2 d3|a6 b5|c4 a6|c8 d7|a6 c8|a8 b7|"
    "c8 a6|e6 f5|g4 e6|e6 c8|e8 d7|c8 e6|e6 c4|c6 d5|c4 e6|f7 d5|"
    "e2 f3|g6 f5|f3 g4|h7 g6|g4 e6|d5 e4|d3 f5|f5 h7|g8 f7|e6 g8"
).split("|")


def test_stage7_white_wins_when_black_has_no_pieces() -> None:
    output = run_game(WINNING_GAME)
    assert WINNER_RE.search(output), (
        "After this complete game, Black has no pieces. Expected an announcement "
        "such as 'White wins' or 'Winner: White'.\n"
        f"Last output:\n{output[-3000:]}"
    )
