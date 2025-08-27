import json
import base64
from io import BytesIO
from typing import Union

from google.protobuf.json_format import Parse
# pylint: disable=no-name-in-module, import-error
from com.daml.ledger.api.v2.interactive.interactive_submission_service_pb2 import \
    PrepareSubmissionResponse # type: ignore

from .canton_utils import read, read_uint, read_varint, write_varint, UINT64_MAX

# from proto.message_pb2 import SimpleInt

class TransactionError(Exception):
    pass


class Transaction:
    def __init__(self,
                 nonce: int,
                 to: Union[str, bytes],
                 value: int,
                 memo: str) -> None:
        self.nonce: int = nonce
        self.to: bytes = bytes.fromhex(to[2:]) if isinstance(to, str) else to
        self.value: int = value
        self.memo: bytes = memo.encode("ascii")

        if not 0 <= self.nonce <= UINT64_MAX:
            raise TransactionError(f"Bad nonce: '{self.nonce}'!")

        if not 0 <= self.value <= UINT64_MAX:
            raise TransactionError(f"Bad value: '{self.value}'!")

        if len(self.to) != 20:
            raise TransactionError(f"Bad address: '{self.to.hex()}'!")

    def serialize(self) -> bytes:
        return b"".join([
            self.nonce.to_bytes(8, byteorder="big"),
            self.to,
            self.value.to_bytes(8, byteorder="big"),
            write_varint(len(self.memo)),
            self.memo
        ])

    @classmethod
    def from_bytes(cls, hexa: Union[bytes, BytesIO]):
        buf: BytesIO = BytesIO(hexa) if isinstance(hexa, bytes) else hexa

        nonce: int = read_uint(buf, 64, byteorder="big")
        to: bytes = read(buf, 20)
        value: int = read_uint(buf, 64, byteorder="big")
        memo_len: int = read_varint(buf)
        memo: str = read(buf, memo_len).decode("ascii")

        return cls(nonce=nonce, to=to, value=value, memo=memo)

    @classmethod
    def serialize_from_json(cls, json_file: str) -> bytes:
        with open(json_file, "r", encoding="utf-8") as file:
            data = json.load(file)

        prepared_tx = PrepareSubmissionResponse()
        Parse(json.dumps(data), prepared_tx)

        return prepared_tx.SerializeToString()

    @classmethod
    def get_hash_from_json(cls, json_file: str) -> bytes:
        with open(json_file, "r", encoding="utf-8") as file:
            data = json.load(file)
        return base64.b64decode(data["prepared_transaction_hash"])
