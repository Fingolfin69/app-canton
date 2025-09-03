import json
import base64
import hashlib
from io import BytesIO
from typing import Union

from google.protobuf.json_format import Parse

# pylint: disable=no-name-in-module, import-error
from com.daml.ledger.api.v2.interactive.interactive_submission_service_pb2 import (
    PrepareSubmissionResponse,
    DamlTransaction,
    Metadata,
)  # type: ignore


from .canton_utils import read, read_uint, read_varint, write_varint, UINT64_MAX

PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE = 11
PURPOSE_PUBLIC_KEY_FINGERPRINT = 12
PURPOSE_MULTI_TOPOLOGY_TRANSACTION = 55


class TransactionError(Exception):
    pass


class Transaction:
    def __init__(self, nonce: int, to: Union[str, bytes], value: int, memo: str) -> None:
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
        return b"".join(
            [
                self.nonce.to_bytes(8, byteorder="big"),
                self.to,
                self.value.to_bytes(8, byteorder="big"),
                write_varint(len(self.memo)),
                self.memo,
            ]
        )

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

    @classmethod
    def serialize_from_json_into_tx_parts(cls, json_file: str) -> tuple[bytes, list[bytes], bytes, list[bytes], bytes]:
        with open(json_file, "r", encoding="utf-8") as file:
            json_tx = json.load(file)

        daml_tx_data, nodes_pb = cls._process_daml_transaction(json_tx["prepared_transaction"]["transaction"])
        metadata_data, input_contracts_pb = cls._process_metadata(json_tx["prepared_transaction"]["metadata"])
        prep_sub_resp_data = cls._process_prep_submission_response(json_tx)

        return (
            daml_tx_data,
            nodes_pb,
            metadata_data,
            input_contracts_pb,
            prep_sub_resp_data,
        )

    @classmethod
    def compute_sha256_canton_hash(cls, purpose: int, content: bytes):
        hash_purpose = purpose.to_bytes(4, byteorder="big")
        # Hashed content
        hashed_content = hashlib.sha256(hash_purpose + content).digest()

        # Multi-hash encoding
        # Canton uses an implementation of multihash (https://github.com/multiformats/multihash)
        # Since we use sha256 always here, we can just hardcode the prefixes
        # This may be improved and simplified in subsequent versions
        sha256_algorithm_prefix = bytes([0x12])
        sha256_length_prefix = bytes([0x20])

        print(f"\n %%%%% Canton Hash {(sha256_algorithm_prefix + sha256_length_prefix + hashed_content).hex()}")

        return sha256_algorithm_prefix + sha256_length_prefix + hashed_content

    @classmethod
    def compute_topology_transaction_hash(cls, serialized_versioned_transaction: bytes) -> bytes:
        """
        Computes the hash of a serialized topology transaction.

        Args:
            serialized_versioned_transaction (bytes): The serialized transaction data.

        Returns:
            bytes: The computed hash.
        """
        print(
            "\n>>>>Computing topology transaction hash for serialized transaction:",
            serialized_versioned_transaction.hex(),
        )
        return Transaction.compute_sha256_canton_hash(
            PURPOSE_TOPOLOGY_TRANSACTION_SIGNATURE, serialized_versioned_transaction
        )

    @classmethod
    def compute_multi_transaction_hash(cls, hashes: list[bytes]) -> bytes:
        """
        Computes a combined hash for multiple topology transactions.

        This function sorts the given hashes, concatenates them with length encoding,
        and computes a Canton-specific SHA-256 hash with a predefined purpose.

        Args:
            hashes (list[bytes]): A list of hashes representing individual topology transactions.

        Returns:
            bytes: The computed multi-transaction hash.
        """
        # Sort the hashes by their hex representation
        sorted_hashes = sorted(hashes, key=lambda h: h.hex())

        print("\nSorted hashes for multi-transaction hash computation:")
        for h in sorted_hashes:
            print(h.hex())

        # Start with the number of hashes encoded as a 4 bytes integer in big endian
        combined_hashes = len(sorted_hashes).to_bytes(4, byteorder="big")

        # Concatenate each hash, prefixing them with their size as a 4 bytes integer in big endian
        for h in sorted_hashes:
            combined_hashes += len(h).to_bytes(4, byteorder="big") + h

        print(f"\nConcatenated sorted hashes for multi-transaction hash computation: {combined_hashes.hex()}")

        return Transaction.compute_sha256_canton_hash(PURPOSE_MULTI_TOPOLOGY_TRANSACTION, combined_hashes)

    @classmethod
    def _process_daml_transaction(cls, daml_tx: dict) -> tuple[bytes, list[bytes]]:
        """Process DAML transaction and its nodes."""
        nodes = daml_tx.pop("nodes", [])
        daml_tx["nodes_count"] = len(nodes)

        daml_tx_pb = DamlTransaction()
        Parse(json.dumps(daml_tx), daml_tx_pb)

        nodes_pb = []
        for node in nodes:
            node_pb = DamlTransaction.Node()
            Parse(json.dumps(node), node_pb)
            nodes_pb.append(node_pb.SerializeToString())

        return daml_tx_pb.SerializeToString(), nodes_pb

    @classmethod
    def _process_metadata(cls, metadata: dict) -> tuple[bytes, list[bytes]]:
        """Process metadata and input contracts."""
        input_contracts = metadata.pop("inputContracts", [])
        metadata["input_contracts_count"] = len(input_contracts)

        metadata_pb = Metadata()
        Parse(json.dumps(metadata), metadata_pb)

        input_contracts_pb = []
        for contract in input_contracts:
            contract_pb = Metadata.InputContract()
            Parse(json.dumps(contract), contract_pb)
            input_contracts_pb.append(contract_pb.SerializeToString())

        return metadata_pb.SerializeToString(), input_contracts_pb

    @classmethod
    def _process_prep_submission_response(cls, json_tx: dict) -> bytes:
        """Process preparation submission response."""
        prep_sub_resp = json_tx.copy()
        del prep_sub_resp["prepared_transaction"]

        prep_sub_resp_pb = PrepareSubmissionResponse()
        Parse(json.dumps(prep_sub_resp), prep_sub_resp_pb)

        return prep_sub_resp_pb.SerializeToString()
