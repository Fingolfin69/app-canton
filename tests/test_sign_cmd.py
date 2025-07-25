import pytest
from ragger.backend.interface import BackendInterface
from ragger.error import ExceptionRAPDU
from ragger.navigator.navigation_scenario import NavigateWithScenario

from application_client.boilerplate_transaction import Transaction
from application_client.boilerplate_command_sender import BoilerplateCommandSender, Errors
from application_client.boilerplate_response_unpacker import unpack_get_public_key_response, unpack_sign_tx_response
from utils import check_signature_validity

# In this test se send to the device a transaction to sign and validate it on screen
# This test is mostly the same as the previous one but with different values.
# In particular the long memo will force the transaction to be sent in multiple chunks
def test_sign_tx_long_tx(backend: BackendInterface, scenario_navigator: NavigateWithScenario) -> None:
    # Use the app interface instead of raw interface
    client = BoilerplateCommandSender(backend)
    path: str = "m/44'/1'/0'/0/0"

    # rapdu = client.get_public_key(path=path)
    # _, public_key, _, _ = unpack_get_public_key_response(rapdu.data)
    
    serialized_tx = Transaction.serialize_from_json("tests/tx_examples/external_sign_ping.json")
    
    print(f"Serialized transaction length: {len(serialized_tx)} bytes")
    
    # client.sign_tx(path=path, transaction=serialized_tx)

    with client.sign_tx(path=path, transaction=serialized_tx) as response:
        print("Transaction sent")
    #     if response.status != 0x9000:
    #         raise ExceptionRAPDU(f"Unexpected status code: {response.status}")
    # #     scenario_navigator.review_approve()

    # response = client.get_async_response().data
    # _, der_sig, _ = unpack_sign_tx_response(response)
    # assert check_signature_validity(public_key, der_sig, transaction)
