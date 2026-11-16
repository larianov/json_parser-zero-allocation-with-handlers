"""Usage: python3 tests/departure_board_tests.py build/departure_board"""
import json
from pathlib import Path
import subprocess
import sys
import tempfile

binary = Path(sys.argv[1]).resolve()
root = Path(__file__).resolve().parents[1]
fixture = root / "examples/enschede-departures.json"
expected = (
    "Enschede | 2026-12-15 | morning\n"
    "SYNTHETIC EXAMPLE: times are unverified; not for travel planning.\n\n"
    "Departure  Destination         Service  Arrival\n"
    "07:40      Hengelo             -        -\n"
    "07:56      Dortmund Hbf        RB51     10:06\n"
)
checks = 0


def check(args, status=0, output=None):
    global checks
    checks += 1
    result = subprocess.run([str(binary), *map(str, args)], cwd=root,
                            capture_output=True, text=True)
    assert result.returncode == status, (args, result.returncode, result.stderr)
    if output is not None:
        assert result.stdout == output, (args, repr(result.stdout))
    if status:
        assert result.stderr and not result.stdout, (args, result.stdout, result.stderr)
    else:
        assert not result.stderr, result.stderr


check([], output=expected)
check([fixture], output=expected)
check([fixture, "extra"], 2)
with tempfile.TemporaryDirectory() as directory:
    directory = Path(directory)
    check([directory / "missing.json"], 1)
    check([directory], 1)
    for name, contents in {
        "empty": "",
        "truncated": '{"board":',
        "missing_metadata": '{"board":{"departures":{}}}',
        "missing_departure": '{"board":{"station":"Enschede","date":"2026-12-15","period":"morning","departures":{"a":{"destination":"Dortmund Hbf"}}}}',
    }.items():
        path = directory / (name + ".json")
        path.write_text(contents)
        check([path], 1)
    scoped = json.loads(fixture.read_text())
    scoped["ignored"] = {"board": {"station": "Wrong station"}}
    scoped["board"]["ignored"] = [{"station": "Wrong station"}]
    scoped["board"]["departures"]["dortmund"]["ignored"] = {
        "destination": "Wrong destination", "nested": [1, 2, 3]
    }
    path = directory / "scope.json"
    path.write_text(json.dumps(scoped))
    check([path], output=expected)
    data = json.loads(fixture.read_text())
    row = data["board"]["departures"]["dortmund"]
    data["board"]["departures"] = {str(i): row for i in range(33)}
    path = directory / "too_many.json"
    path.write_text(json.dumps(data))
    check([path], 1)
    data["board"]["departures"].pop("32")
    path.write_text(json.dumps(data))
    check([path])
print(f"All {checks} departure-board smoke tests passed")
