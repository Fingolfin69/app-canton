from pathlib import Path
from ragger.backend.interface import BackendInterface
# from ragger.error import ExceptionRAPDU
from ragger.navigator.navigation_scenario import NavigateWithScenario

from application_client.canton_transaction import Transaction
from application_client.canton_command_sender import (
    BoilerplateCommandSender,
    P1SignType,
    # Errors,
)
from application_client.canton_response_unpacker import (
    unpack_get_public_key_response,
    unpack_sign_tx_response,
)
from utils import verify_signature

ROOT_SCREENSHOT_PATH = Path(__file__).parent.resolve()

def test_sign_tx_hash(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    tx_hash = Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )

    with client.sign_tx(path=path, transaction=tx_hash, p1=P1SignType.P1_SIGN_HASH):
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH, test_name="test_sign_tx_hash")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)


# In this test se send to the device a transaction to sign and validate it on screen
# This test is mostly the same as the previous one but with different values.
# In particular the long memo will force the transaction to be sent in multiple chunks
def test_sign_tx_ping(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    serialized_tx = Transaction.serialize_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )

    tx_hash = Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )
    print(f"Transaction hash: {tx_hash.hex()}")

    print(f"Serialized transaction length: {len(serialized_tx)} bytes")

    with client.sign_tx(path=path, transaction=serialized_tx, p1=P1SignType.P1_SIGN_PREPARED_TRANSACTION):
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH, test_name="test_sign_tx_ping")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)
