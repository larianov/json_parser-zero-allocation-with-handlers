"""Standard-library-only smoke tests. Usage: python3 tests/cli_tests.py build/MyProject"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

binary = Path(sys.argv[1]).resolve()
fixture = Path(__file__).resolve().parents[1] / "examples/enschede-departures.json"


checks = 0


def check(args, status=0, output=None):
    global checks
    checks += 1
    result = subprocess.run([str(binary), *map(str, args)], capture_output=True, text=True)
    assert result.returncode == status, (args, result.returncode, result.stderr)
    if output is not None:
        assert result.stdout == output, (args, result.stdout)
    if status:
        assert result.stderr, args
        assert not result.stdout, (args, result.stdout)
    else:
        assert not result.stderr, (args, result.stderr)


board = json.loads(fixture.read_text())["board"]
assert board["data_kind"] == "synthetic example, not a verified timetable"
for suffix, value in {
    "destination": "Dortmund Hbf",
    "service": "RB51",
    "departure": "07:56",
    "arrival": "10:06",
    "delay_minutes": "0",
    "platform": "null",
    "cancelled": "false",
}.items():
    check([fixture, "board.departures.dortmund." + suffix], output=value + "\n")
check([fixture, "board.departures.hengelo.destination"], output="Hengelo\n")
check([fixture, "board.station"], output="Enschede\n")
check([fixture, "board.date"], output="2026-12-15\n")
check([fixture, "board.missing"], 1)
check([fixture, "board.departures"], 1)
check([], 2)
check([fixture], 2)
check([fixture, "board.station", "extra"], 2)
for path in ["", ".board", "board.", "board..station", ".".join(["key"] * 65)]:
    check([fixture, path], 2)
with tempfile.TemporaryDirectory() as directory:
    directory = Path(directory)
    check([directory / "missing.json", "target"], 1)
    check([directory, "target"], 1)
    for name, contents in {
        "empty.json": "",
        "truncated.json": '{"target":"matched but incomplete"',
        "invalid.json": '{"target":?}',
    }.items():
        path = directory / name
        path.write_text(contents)
        check([path, "target"], 1)
print(f"All {checks} CLI smoke tests passed")
