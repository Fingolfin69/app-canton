from enum import IntEnum
from typing import Generator, List, Optional
from contextlib import contextmanager

from ragger.backend.interface import BackendInterface, RAPDU
from ragger.bip import pack_derivation_path


MAX_APDU_LEN: int = 255

CLA: int = 0xE0

class P1(IntEnum):
    P1_NONE = 0x00
    P1_CONFIRM = 0x01

class P1SignType(IntEnum):
    P1_SIGN_HASH = 0x00
    P1_SIGN_UNTYPED_VERSIONED_MESSAGE = 0x01
    P1_SIGN_PREPARED_TRANSACTION = 0x02

class P2(IntEnum):
    P2_NONE = 0x00
    P2_FIRST = 0x01
    P2_MORE = 0x02
    P2_MSG_END = 0x04

class InsType(IntEnum):
    GET_VERSION    = 0x03
    GET_APP_NAME   = 0x04
    GET_PUBLIC_KEY = 0x05
    SIGN_TX        = 0x06

class Errors(IntEnum):
    SW_DENY                    = 0x6985
    SW_WRONG_P1P2              = 0x6A86
    SW_WRONG_DATA_LENGTH       = 0x6A87
    SW_INS_NOT_SUPPORTED       = 0x6D00
    SW_CLA_NOT_SUPPORTED       = 0x6E00
    SW_WRONG_RESPONSE_LENGTH   = 0xB000
    SW_DISPLAY_BIP32_PATH_FAIL = 0xB001
    SW_DISPLAY_ADDRESS_FAIL    = 0xB002
    SW_DISPLAY_AMOUNT_FAIL     = 0xB003
    SW_WRONG_TX_LENGTH         = 0xB004
    SW_TX_PARSING_FAIL         = 0xB005
    SW_TX_HASH_FAIL            = 0xB006
    SW_BAD_STATE               = 0xB007
    SW_SIGNATURE_FAIL          = 0xB008


def split_message(message: bytes, max_size: int) -> List[bytes]:
    return [message[x:x + max_size] for x in range(0, len(message), max_size)]


class BoilerplateCommandSender:
    def __init__(self, backend: BackendInterface) -> None:
        self.backend = backend


    def get_app_and_version(self) -> RAPDU:
        return self.backend.exchange(cla=0xB0,  # specific CLA for BOLOS
                                     ins=0x01,  # specific INS for get_app_and_version
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_version(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_VERSION,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_app_name(self) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_APP_NAME,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=b"")


    def get_public_key(self, path: str) -> RAPDU:
        return self.backend.exchange(cla=CLA,
                                     ins=InsType.GET_PUBLIC_KEY,
                                     p1=P1.P1_NONE,
                                     p2=P2.P2_NONE,
                                     data=pack_derivation_path(path))


    @contextmanager
    def get_public_key_with_confirmation(self, path: str) -> Generator[None, None, None]:
        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.GET_PUBLIC_KEY,
                                         p1=P1.P1_CONFIRM,
                                         p2=P2.P2_NONE,
                                         data=pack_derivation_path(path)) as response:
            yield response


    @contextmanager
    def sign_tx(self, path: str, transaction: bytes, p1: P1SignType) -> Generator[None, None, None]:
        print(f"Signing transaction with path: {path} and transaction length: {len(transaction)} bytes")
        self.backend.exchange(cla=CLA,
                              ins=InsType.SIGN_TX,
                              p1=p1,
                              p2=P2.P2_FIRST | P2.P2_MORE,
                              data=pack_derivation_path(path))
        messages = split_message(transaction, MAX_APDU_LEN)

        print(f"Sending {len(messages)} chunks of transaction data")

        for msg in messages[:-1]:
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_TX,
                                  p1=p1,
                                  p2=P2.P2_MORE,
                                  data=msg)

        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.SIGN_TX,
                                         p1=p1,
                                         p2=P2.P2_NONE,
                                         data=messages[-1]) as response:
            yield response

    @contextmanager
    def sign_tx_in_parts(
        self,
        path: str,
        daml_transaction: bytes,
        nodes: list[bytes],
        metadata: bytes,
        input_contracts: list[bytes],
        prepared_submission_details: bytes
    ) -> Generator[None, None, None]:
        print(f"Signing transaction (in parts) with path: {path}")
        p1 = P1SignType.P1_SIGN_PREPARED_TRANSACTION,

        self.backend.exchange(cla=CLA,
                              ins=InsType.SIGN_TX,
                              p1=p1,
                              p2=P2.P2_FIRST | P2.P2_MORE,
                              data=pack_derivation_path(path))

        daml_tx_messages = split_message(daml_transaction, MAX_APDU_LEN)

        print(f"Sending {len(daml_tx_messages)} chunks of DamlTransaction data")

        for chunk_id, msg in enumerate(daml_tx_messages[:-1], start=1):
            print(f"Sending chunk {chunk_id} of {len(daml_tx_messages)}")
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_TX,
                                  p1=p1,
                                  p2=P2.P2_MORE,
                                  data=msg)

        self.backend.exchange(cla=CLA,
                                ins=InsType.SIGN_TX,
                                p1=p1,
                                p2=P2.P2_MORE | P2.P2_MSG_END,
                                data=daml_tx_messages[-1])

        print(f"Sending {len(nodes)} Nodes")

        for node_id, node in enumerate(nodes):
            print(f"Sending node  {node_id} of {len(nodes)}")
            messages = split_message(node, MAX_APDU_LEN)
            for chunk_id, msg in enumerate(messages[:-1], start=1):
                print(f"Sending node chunk {chunk_id} of {len(messages)}")
                self.backend.exchange(cla=CLA,
                                      ins=InsType.SIGN_TX,
                                      p1=p1,
                                      p2=P2.P2_MORE,
                                      data=msg)

            self.backend.exchange(cla=CLA,
                                    ins=InsType.SIGN_TX,
                                    p1=p1,
                                    p2=P2.P2_MORE | P2.P2_MSG_END,
                                    data=messages[-1])


        metadata_messages = split_message(metadata, MAX_APDU_LEN)
        print(f"Sending {len(metadata_messages)} chunks of Metadata data")

        for chunk_id, msg in enumerate(metadata_messages[:-1], start=1):
            print(f"Sending metadata chunk {chunk_id} of {len(metadata_messages)}")
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_TX,
                                  p1=p1,
                                  p2=P2.P2_MORE,
                                  data=msg)

        self.backend.exchange(cla=CLA,
                                ins=InsType.SIGN_TX,
                                p1=p1,
                                p2=P2.P2_MORE | P2.P2_MSG_END,
                                data=metadata_messages[-1])


        print(f"Sending {len(input_contracts)} InputContracts")
        for input_contract in input_contracts:
            messages = split_message(input_contract, MAX_APDU_LEN)
            for chunk_id, msg in enumerate(messages[:-1], start=1):
                print(f"Sending input contract chunk {chunk_id} of {len(messages)}")
                self.backend.exchange(cla=CLA,
                                      ins=InsType.SIGN_TX,
                                      p1=p1,
                                      p2=P2.P2_MORE,
                                      data=msg)

            self.backend.exchange(cla=CLA,
                                    ins=InsType.SIGN_TX,
                                    p1=p1,
                                    p2=P2.P2_MORE | P2.P2_MSG_END,
                                    data=messages[-1])


        ps_messages = split_message(prepared_submission_details, MAX_APDU_LEN)

        print(f"Sending {len(ps_messages)} chunks of PreparedSubmission details data")

        for chunk_id, msg in enumerate(ps_messages[:-1], start=1):
            print(f"Sending chunk {chunk_id} of {len(ps_messages)}")
            self.backend.exchange(cla=CLA,
                                  ins=InsType.SIGN_TX,
                                  p1=p1,
                                  p2=P2.P2_MORE,
                                  data=msg)

        with self.backend.exchange_async(cla=CLA,
                                         ins=InsType.SIGN_TX,
                                         p1=p1,
                                         p2=P2.P2_NONE,
                                         data=ps_messages[-1]) as response:
            yield response

    def get_async_response(self) -> Optional[RAPDU]:
        return self.backend.last_async_response
