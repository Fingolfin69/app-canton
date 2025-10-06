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
from utils import verify_signature

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
    _, der_sig, _ = unpack_sign_tx_response(response)
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
    _, der_sig, _ = unpack_sign_tx_response(response)
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
                   validator_seeds: list[bytes]) -> None:
    path: str = "m/44'/6767'/0'/0'/0'"
    client = CantonCommandSender(backend)
    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    namespace_delegation_tx = Transaction.namespace_delegation(public_key)
    party_to_key_tx = Transaction.party_to_key(public_key)
    party_to_participant_tx = Transaction.party_to_participant(public_key, validator_seeds)

    txs = [namespace_delegation_tx, party_to_key_tx, party_to_participant_tx]

    hashes = [Transaction.compute_topology_transaction_hash(tx) for tx in txs]
    multi_hash = Transaction.compute_multi_transaction_hash(hashes)

    with client.sign_topology_tx(path=path, transactions=txs):
        scenario_navigator.review_approve(path=ROOT_SCREENSHOT_PATH, custom_screen_text="Sign transaction to")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, multi_hash, der_sig)


def test_sign_onboarding_single_validator(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1])

def test_sign_onboarding_three_validators(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    _onboard_party(backend, scenario_navigator, validator_seeds=[VALIDATOR_SEED_1, VALIDATOR_SEED_2, VALIDATOR_SEED_3])
