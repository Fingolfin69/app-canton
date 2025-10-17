import os
from pathlib import Path
from typing import Optional
from ragger.backend.interface import BackendInterface
# from ragger.error import ExceptionRAPDU
from ragger.navigator.navigation_scenario import NavigateWithScenario

from application_client.canton_transaction import Transaction
from application_client.canton_command_sender import (
    CantonCommandSender,
    P1SignType,
    # Errors,
)
from application_client.canton_response_unpacker import (
    unpack_get_public_key_response,
    unpack_sign_tx_response,
)
from utils import (
    verify_signature,
    read_attestation_keys,
)

ROOT_SCREENSHOT_PATH = Path(__file__).parent.resolve()

# 32 bytes seeds for validators
VALIDATOR_SEED_1 = b"validator1______________________"
VALIDATOR_SEED_2 = b"validator2______________________"
VALIDATOR_SEED_3 = b"validator3______________________"

def _sign_and_verify_hash(
    backend: BackendInterface,
    scenario_navigator: NavigateWithScenario,
    tx_hash: bytes,
    test_name: str,
) -> None:
    client = CantonCommandSender(backend)
    path = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    print(f"Public key returned from device: {public_key.hex()}")

    with client.sign_tx(path=path, transaction=tx_hash, p1=P1SignType.P1_SIGN_HASH):
        scenario_navigator.review_approve_with_warning(
            path=ROOT_SCREENSHOT_PATH, test_name=test_name
        )

    response = client.get_async_response().data
    _, der_sig, _, _, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)


def test_sign_hash_32(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    tx_hash = Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )
    _sign_and_verify_hash(
        backend, scenario_navigator, tx_hash, test_name="test_sign_hash_32"
    )


def test_sign_hash_34(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    tx_hash = b"\x00\x01" + Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )
    _sign_and_verify_hash(
        backend, scenario_navigator, tx_hash, test_name="test_sign_hash_34"
    )

def _sign_and_verify_prepared_transaction(
    backend: BackendInterface,
    scenario_navigator: NavigateWithScenario,
    tx_json: str,
    custom_screen_text: Optional[str] = None,
    warning: bool = False,
) -> None:
    client = CantonCommandSender(backend)
    path: str = "m/44'/6767'/0'/0'/0'"

    _, public_key, _, _ = unpack_get_public_key_response(client.get_public_key(path=path).data)

    ser_tx, ser_nodes, ser_meta, ser_contracts = Transaction.serialize_from_json_into_tx_parts(tx_json)
    tx_hash = Transaction.get_hash_from_json(tx_json)
    print(f"Transaction hash: {tx_hash.hex()}")
    print(f"Serialized transaction length: {len(ser_tx) + len(ser_nodes) + len(ser_meta) + len(ser_contracts)} bytes")

    with client.sign_tx_in_parts(path, ser_tx, ser_nodes, ser_meta, ser_contracts) as response:
        if warning:
            scenario_navigator.review_approve_with_warning(
                path=ROOT_SCREENSHOT_PATH, custom_screen_text=custom_screen_text)
        else:
            scenario_navigator.review_approve(
                path=ROOT_SCREENSHOT_PATH, custom_screen_text=custom_screen_text)

    response = client.get_async_response().data
    _, der_sig, _, _, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)

def test_sign_ping(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/external_sign_ping.json",
        warning=True,
    )

def test_sign_native_transfer(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/native_transfer.json",
        custom_screen_text="Sign transaction to",
    )

def test_sign_token_transfer(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/token_transfer.json",
        custom_screen_text="Sign transaction to",
    )

def test_sign_token_transfer_with_memo(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/token_transfer_with_memo.json",
        custom_screen_text="Sign transaction to",
    )

def test_sign_token_transfer_16_node_children(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/token_transfer_big.json",
        custom_screen_text="Sign transaction to",
    )


def test_sign_preapproval_proposal(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    _sign_and_verify_prepared_transaction(
        backend,
        scenario_navigator,
        tx_json="tests/tx_examples/preapproval_proposal.json",
        custom_screen_text="Sign transaction to",
    )

def _onboard_party(backend: BackendInterface,
                   scenario_navigator: NavigateWithScenario,
                   validator_seeds: list[bytes],
                   attestation_keys: Optional[tuple[bytes,bytes]] = None,
                   der_key_format: bool = True) -> None:
    client = CantonCommandSender(backend)

    # Get public key
    rapdu = client.get_public_key(path="m/44'/6767'/0'/0'/0'")
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    raw_key = public_key
    if der_key_format:
        # Convert to DER format for inclusion in topology transactions
        public_key = b"\x30\x2A\x30\x05\x06\x03\x2B\x65\x70\x03\x21\x00" + public_key

    # Create and hash transactions
    txs = [
        Transaction.namespace_delegation(public_key, der_key_format),
        Transaction.party_to_key(public_key, der_key_format),
        Transaction.party_to_participant(public_key, validator_seeds)
    ]
    multi_hash = Transaction.compute_multi_transaction_hash(
        [Transaction.compute_topology_transaction_hash(tx) for tx in txs]
    )

    # Sign transactions
    challenge = os.urandom(24) if attestation_keys else None
    with client.sign_topology_tx(path="m/44'/6767'/0'/0'/0'", transactions=txs, challenge=challenge):
        scenario_navigator.review_approve(path=ROOT_SCREENSHOT_PATH, custom_screen_text="Sign transaction to")

    # Verify signatures
    _, der_sig, _, challenge_sig_len, challenge_sig = unpack_sign_tx_response(
        client.get_async_response().data
    )
    verify_signature(raw_key, multi_hash, der_sig)

    if attestation_keys:
        _verify_attestation(attestation_keys[1], multi_hash, challenge, challenge_sig, challenge_sig_len)
    else:
        assert challenge is None
        assert challenge_sig is None
        assert challenge_sig_len is None


def _verify_attestation(attest_pub_key: bytes, multi_hash: bytes, challenge: Optional[bytes],
                        challenge_sig: Optional[bytes], challenge_sig_len: int | None) -> None:
    assert challenge_sig is not None
    assert challenge_sig_len is not None
    assert challenge_sig_len == 64 == len(challenge_sig)
    assert challenge is not None
    verify_signature(attest_pub_key, multi_hash + challenge, challenge_sig)

def test_sign_onboarding_attested(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    path: Path = Path(__file__).parent.parent / "src" / "crypto_data.h"
    attest_key, attest_pub_key = read_attestation_keys(path)
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1],
                   attestation_keys=(attest_key, attest_pub_key))

def test_sign_onboarding_raw_format_key(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1], der_key_format=False)

def test_sign_onboarding_single_validator(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1])

def test_sign_onboarding_three_validators(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1, VALIDATOR_SEED_2, VALIDATOR_SEED_3])
