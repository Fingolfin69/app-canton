from pathlib import Path
from typing import List
import re
from nacl.signing import VerifyKey


def verify_signature(from_public_key: bytes, message: bytes, signature: bytes):
    print("Sig len :", len(signature))
    assert len(signature) == 64, "signature size incorrect"
    verify_key = VerifyKey(from_public_key)
    verify_key.verify(message, signature)


def verify_name(name: str) -> None:
    """Verify the app name, based on defines in Makefile

    Args:
        name (str): Name to be checked
    """

    name_str = ""
    lines = _read_makefile()
    name_re = re.compile(r"^APPNAME\s?=\s?\"?(?P<val>\w+)\"?", re.I)
    for line in lines:
        info = name_re.match(line)
        if info:
            dinfo = info.groupdict()
            name_str = dinfo["val"]
    assert name == name_str


def verify_version(version: str) -> None:
    """Verify the app version, based on defines in Makefile

    Args:
        Version (str): Version to be checked
    """

    vers_dict = {}
    vers_str = ""
    lines = _read_makefile()
    version_re = re.compile(r"^APPVERSION_(?P<part>\w)\s?=\s?(?P<val>\d*)", re.I)
    for line in lines:
        info = version_re.match(line)
        if info:
            dinfo = info.groupdict()
            vers_dict[dinfo["part"]] = dinfo["val"]
    try:
        vers_str = f"{vers_dict['M']}.{vers_dict['N']}.{vers_dict['P']}"
    except KeyError:
        pass
    assert version == vers_str


def _read_makefile() -> List[str]:
    """Read lines from the parent Makefile"""

    parent = Path(__file__).parent.parent.resolve()
    makefile = f"{parent}/Makefile"
    with open(makefile, "r", encoding="utf-8") as f_p:
        lines = f_p.readlines()
    return lines


def _read_key_block(lines: list[str], start_keyword: str, end_keyword: str = "};") -> bytes:
    key_bytes = b""
    found_key = False
    for line in lines:
        if start_keyword in line:
            found_key = True
            continue
        if found_key:
            # Extract hex values from the line
            hex_values = line.strip().strip("{};,").split(",")
            for hv in hex_values:
                hv = hv.strip()
                if hv.startswith("0x"):
                    key_bytes += bytes([int(hv, 16)])
            if end_keyword in line:
                break

    return key_bytes

def read_attestation_keys(filename: Path) -> tuple[bytes, bytes]:
    print(f"Reading attestation keys from: {filename.resolve()}")
    with open(filename, "r", encoding="utf-8") as f:
        lines = f.readlines()
        priv_key = _read_key_block(lines, "TEST_ATTESTATION_KEY")
        pub_key = _read_key_block(lines, "TEST_ATTESTATION_PUBKEY")

        print(f"Private key: {priv_key.hex()}")
        print(f"Public key: {pub_key.hex()}")
        return priv_key, pub_key
