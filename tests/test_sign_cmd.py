from pathlib import Path
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

def test_sign_tx_hash_32(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    # Use the app interface instead of raw interface
    client = CantonCommandSender(backend)
    path = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    tx_hash = Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )

    with client.sign_tx(path=path, transaction=tx_hash, p1=P1SignType.P1_SIGN_HASH):
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH, test_name="test_sign_tx_hash_32")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)


def test_sign_tx_hash_34(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    # Use the app interface instead of raw interface
    client = CantonCommandSender(backend)
    path = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    tx_hash = b"\x00\x01" + Transaction.get_hash_from_json(
        "tests/tx_examples/external_sign_ping.json"
    )

    with client.sign_tx(path=path, transaction=tx_hash, p1=P1SignType.P1_SIGN_HASH):
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH, test_name="test_sign_tx_hash_34")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)

# In this test se send to the device a transaction to sign and validate it on screen
# This test is mostly the same as the previous one but with different values.
# In particular the long memo will force the transaction to be sent in multiple chunks
def test_sign_tx_ping(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    tx_json = "tests/tx_examples/external_sign_ping.json"

    # Use the app interface instead of raw interface
    client = CantonCommandSender(backend)
    path: str = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    (ser_tx, ser_nodes, ser_meta, ser_contracts) = Transaction.serialize_from_json_into_tx_parts(tx_json)

    tx_hash = Transaction.get_hash_from_json(tx_json)
    print(f"Transaction hash: {tx_hash.hex()}")
    total_len = len(ser_tx) + len(ser_nodes) + len(ser_meta) + len(ser_contracts)
    print(f"Serialized transaction length: {total_len} bytes")

    with client.sign_tx_in_parts(path, ser_tx, ser_nodes, ser_meta, ser_contracts) as response:
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH)

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)


def test_sign_tx_token_transfer(
    backend: BackendInterface, scenario_navigator: NavigateWithScenario
) -> None:
    tx_json = "tests/tx_examples/token_transfer.json"

    # Use the app interface instead of raw interface
    client = CantonCommandSender(backend)
    path: str = "m/44'/6767'/0'/0'/0'"

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    (ser_tx, ser_nodes, ser_meta, ser_contracts) = Transaction.serialize_from_json_into_tx_parts(tx_json)

    tx_hash = Transaction.get_hash_from_json(tx_json)
    print(f"Transaction hash: {tx_hash.hex()}")
    total_len = len(ser_tx) + len(ser_nodes) + len(ser_meta) + len(ser_contracts)
    print(f"Serialized transaction length: {total_len} bytes")

    with client.sign_tx_in_parts(path, ser_tx, ser_nodes, ser_meta, ser_contracts) as response:
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH)

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, tx_hash, der_sig)


def test_sign_topology_tx(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    # Use the app interface instead of raw interface
    client = CantonCommandSender(backend)
    path = "m/44'/6767'/0'/0'/0'"

    namespace_delegation_tx = (
        "0a7e080110011a780a760a443132323062366665623630366665373264653936303634613333323232613962356335386361"
        "363135323662356161623761613231366537646236636437363336343037122c10031a20ff2d9c046d1fdf68e65ecd36278"
        "eb8ead12aa38ef5b323769baa5df9216f85d820012a02010430011801101e"
    )

    party_to_key_tx = (
        "0a8401080110021a7e82017b0a49626f623a3a31323230623666656236303666653732646539363036346133333232326139"
        "623563353863613631353236623561616237616132313665376462366364373633363430371801222c10031a20ff2d9c046d"
        "1fdf68e65ecd36278eb8ead12aa38ef5b323769baa5df9216f85d820012a0201043001101e"
    )

    party_to_participant_tx = (
        "0aae01080110011aa7014aa4010a49626f623a3a313232306236666562363036666537326465393630363461333332323261"
        "396235633538636136313532366235616162376161323136653764623663643736333634303710011a550a51706172746963"
        "6970616e743a3a31323230313235366361636138616135343236343436643262633461313632393163343862373537326238"
        "616262306431623365656136336364323939363732316362321002101e"
    )

    txs = [namespace_delegation_tx, party_to_key_tx, party_to_participant_tx]

    rapdu = client.get_public_key(path=path)
    _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)

    hashes = [Transaction.compute_topology_transaction_hash(bytes.fromhex(tx)) for tx in txs]
    multi_hash = Transaction.compute_multi_transaction_hash(hashes)

    with client.sign_topology_tx(path=path, transactions=[bytes.fromhex(tx) for tx in txs]):
        scenario_navigator.review_approve_with_warning(path=ROOT_SCREENSHOT_PATH, test_name="test_sign_topology_tx")

    response = client.get_async_response().data
    _, der_sig, _ = unpack_sign_tx_response(response)
    verify_signature(public_key, multi_hash, der_sig)
