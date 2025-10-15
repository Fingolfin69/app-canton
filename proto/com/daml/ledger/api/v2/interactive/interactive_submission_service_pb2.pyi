from com.daml.ledger.api.v2 import commands_pb2 as _commands_pb2
from com.daml.ledger.api.v2.interactive import interactive_submission_common_data_pb2 as _interactive_submission_common_data_pb2
from com.daml.ledger.api.v2.interactive.transaction.v1 import interactive_submission_data_pb2 as _interactive_submission_data_pb2
from com.daml.ledger.api.v2 import package_reference_pb2 as _package_reference_pb2
from com.daml.ledger.api.v2 import value_pb2 as _value_pb2
from google.protobuf import duration_pb2 as _duration_pb2
from google.protobuf import timestamp_pb2 as _timestamp_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor
HASHING_SCHEME_VERSION_UNSPECIFIED: HashingSchemeVersion
HASHING_SCHEME_VERSION_V2: HashingSchemeVersion
SIGNATURE_FORMAT_CONCAT: SignatureFormat
SIGNATURE_FORMAT_DER: SignatureFormat
SIGNATURE_FORMAT_RAW: SignatureFormat
SIGNATURE_FORMAT_SYMBOLIC: SignatureFormat
SIGNATURE_FORMAT_UNSPECIFIED: SignatureFormat
SIGNING_ALGORITHM_SPEC_EC_DSA_SHA_256: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_EC_DSA_SHA_384: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_ED25519: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_UNSPECIFIED: SigningAlgorithmSpec

class DamlTransaction(_message.Message):
    __slots__ = ["node_seeds", "nodes", "roots", "version"]
    class Node(_message.Message):
        __slots__ = ["node_id", "v1"]
        NODE_ID_FIELD_NUMBER: _ClassVar[int]
        V1_FIELD_NUMBER: _ClassVar[int]
        node_id: str
        v1: _interactive_submission_data_pb2.Node
        def __init__(self, node_id: _Optional[str] = ..., v1: _Optional[_Union[_interactive_submission_data_pb2.Node, _Mapping]] = ...) -> None: ...
    class NodeSeed(_message.Message):
        __slots__ = ["node_id", "seed"]
        NODE_ID_FIELD_NUMBER: _ClassVar[int]
        SEED_FIELD_NUMBER: _ClassVar[int]
        node_id: int
        seed: bytes
        def __init__(self, node_id: _Optional[int] = ..., seed: _Optional[bytes] = ...) -> None: ...
    NODES_FIELD_NUMBER: _ClassVar[int]
    NODE_SEEDS_FIELD_NUMBER: _ClassVar[int]
    ROOTS_FIELD_NUMBER: _ClassVar[int]
    VERSION_FIELD_NUMBER: _ClassVar[int]
    node_seeds: _containers.RepeatedCompositeFieldContainer[DamlTransaction.NodeSeed]
    nodes: _containers.RepeatedCompositeFieldContainer[DamlTransaction.Node]
    roots: _containers.RepeatedScalarFieldContainer[str]
    version: str
    def __init__(self, version: _Optional[str] = ..., roots: _Optional[_Iterable[str]] = ..., nodes: _Optional[_Iterable[_Union[DamlTransaction.Node, _Mapping]]] = ..., node_seeds: _Optional[_Iterable[_Union[DamlTransaction.NodeSeed, _Mapping]]] = ...) -> None: ...

class ExecuteSubmissionRequest(_message.Message):
    __slots__ = ["deduplication_duration", "deduplication_offset", "hashing_scheme_version", "min_ledger_time", "party_signatures", "prepared_transaction", "submission_id", "user_id"]
    DEDUPLICATION_DURATION_FIELD_NUMBER: _ClassVar[int]
    DEDUPLICATION_OFFSET_FIELD_NUMBER: _ClassVar[int]
    HASHING_SCHEME_VERSION_FIELD_NUMBER: _ClassVar[int]
    MIN_LEDGER_TIME_FIELD_NUMBER: _ClassVar[int]
    PARTY_SIGNATURES_FIELD_NUMBER: _ClassVar[int]
    PREPARED_TRANSACTION_FIELD_NUMBER: _ClassVar[int]
    SUBMISSION_ID_FIELD_NUMBER: _ClassVar[int]
    USER_ID_FIELD_NUMBER: _ClassVar[int]
    deduplication_duration: _duration_pb2.Duration
    deduplication_offset: int
    hashing_scheme_version: HashingSchemeVersion
    min_ledger_time: MinLedgerTime
    party_signatures: PartySignatures
    prepared_transaction: PreparedTransaction
    submission_id: str
    user_id: str
    def __init__(self, prepared_transaction: _Optional[_Union[PreparedTransaction, _Mapping]] = ..., party_signatures: _Optional[_Union[PartySignatures, _Mapping]] = ..., deduplication_duration: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., deduplication_offset: _Optional[int] = ..., submission_id: _Optional[str] = ..., user_id: _Optional[str] = ..., hashing_scheme_version: _Optional[_Union[HashingSchemeVersion, str]] = ..., min_ledger_time: _Optional[_Union[MinLedgerTime, _Mapping]] = ...) -> None: ...

class ExecuteSubmissionResponse(_message.Message):
    __slots__ = []
    def __init__(self) -> None: ...

class GetPreferredPackageVersionRequest(_message.Message):
    __slots__ = ["package_name", "parties", "synchronizer_id", "vetting_valid_at"]
    PACKAGE_NAME_FIELD_NUMBER: _ClassVar[int]
    PARTIES_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    VETTING_VALID_AT_FIELD_NUMBER: _ClassVar[int]
    package_name: str
    parties: _containers.RepeatedScalarFieldContainer[str]
    synchronizer_id: str
    vetting_valid_at: _timestamp_pb2.Timestamp
    def __init__(self, parties: _Optional[_Iterable[str]] = ..., package_name: _Optional[str] = ..., synchronizer_id: _Optional[str] = ..., vetting_valid_at: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ...) -> None: ...

class GetPreferredPackageVersionResponse(_message.Message):
    __slots__ = ["package_preference"]
    PACKAGE_PREFERENCE_FIELD_NUMBER: _ClassVar[int]
    package_preference: PackagePreference
    def __init__(self, package_preference: _Optional[_Union[PackagePreference, _Mapping]] = ...) -> None: ...

class Metadata(_message.Message):
    __slots__ = ["global_key_mapping", "input_contracts", "max_ledger_effective_time", "mediator_group", "min_ledger_effective_time", "submission_time", "submitter_info", "synchronizer_id", "transaction_uuid"]
    class GlobalKeyMappingEntry(_message.Message):
        __slots__ = ["key", "value"]
        KEY_FIELD_NUMBER: _ClassVar[int]
        VALUE_FIELD_NUMBER: _ClassVar[int]
        key: _interactive_submission_common_data_pb2.GlobalKey
        value: _value_pb2.Value
        def __init__(self, key: _Optional[_Union[_interactive_submission_common_data_pb2.GlobalKey, _Mapping]] = ..., value: _Optional[_Union[_value_pb2.Value, _Mapping]] = ...) -> None: ...
    class InputContract(_message.Message):
        __slots__ = ["created_at", "driver_metadata", "v1"]
        CREATED_AT_FIELD_NUMBER: _ClassVar[int]
        DRIVER_METADATA_FIELD_NUMBER: _ClassVar[int]
        V1_FIELD_NUMBER: _ClassVar[int]
        created_at: int
        driver_metadata: bytes
        v1: _interactive_submission_data_pb2.Create
        def __init__(self, v1: _Optional[_Union[_interactive_submission_data_pb2.Create, _Mapping]] = ..., created_at: _Optional[int] = ..., driver_metadata: _Optional[bytes] = ...) -> None: ...
    class SubmitterInfo(_message.Message):
        __slots__ = ["act_as", "command_id"]
        ACT_AS_FIELD_NUMBER: _ClassVar[int]
        COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
        act_as: _containers.RepeatedScalarFieldContainer[str]
        command_id: str
        def __init__(self, act_as: _Optional[_Iterable[str]] = ..., command_id: _Optional[str] = ...) -> None: ...
    GLOBAL_KEY_MAPPING_FIELD_NUMBER: _ClassVar[int]
    INPUT_CONTRACTS_FIELD_NUMBER: _ClassVar[int]
    MAX_LEDGER_EFFECTIVE_TIME_FIELD_NUMBER: _ClassVar[int]
    MEDIATOR_GROUP_FIELD_NUMBER: _ClassVar[int]
    MIN_LEDGER_EFFECTIVE_TIME_FIELD_NUMBER: _ClassVar[int]
    SUBMISSION_TIME_FIELD_NUMBER: _ClassVar[int]
    SUBMITTER_INFO_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    TRANSACTION_UUID_FIELD_NUMBER: _ClassVar[int]
    global_key_mapping: _containers.RepeatedCompositeFieldContainer[Metadata.GlobalKeyMappingEntry]
    input_contracts: _containers.RepeatedCompositeFieldContainer[Metadata.InputContract]
    max_ledger_effective_time: int
    mediator_group: int
    min_ledger_effective_time: int
    submission_time: int
    submitter_info: Metadata.SubmitterInfo
    synchronizer_id: str
    transaction_uuid: str
    def __init__(self, submitter_info: _Optional[_Union[Metadata.SubmitterInfo, _Mapping]] = ..., synchronizer_id: _Optional[str] = ..., mediator_group: _Optional[int] = ..., transaction_uuid: _Optional[str] = ..., submission_time: _Optional[int] = ..., input_contracts: _Optional[_Iterable[_Union[Metadata.InputContract, _Mapping]]] = ..., min_ledger_effective_time: _Optional[int] = ..., max_ledger_effective_time: _Optional[int] = ..., global_key_mapping: _Optional[_Iterable[_Union[Metadata.GlobalKeyMappingEntry, _Mapping]]] = ...) -> None: ...

class MinLedgerTime(_message.Message):
    __slots__ = ["min_ledger_time_abs", "min_ledger_time_rel"]
    MIN_LEDGER_TIME_ABS_FIELD_NUMBER: _ClassVar[int]
    MIN_LEDGER_TIME_REL_FIELD_NUMBER: _ClassVar[int]
    min_ledger_time_abs: _timestamp_pb2.Timestamp
    min_ledger_time_rel: _duration_pb2.Duration
    def __init__(self, min_ledger_time_abs: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ..., min_ledger_time_rel: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ...) -> None: ...

class PackagePreference(_message.Message):
    __slots__ = ["package_reference", "synchronizer_id"]
    PACKAGE_REFERENCE_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    package_reference: _package_reference_pb2.PackageReference
    synchronizer_id: str
    def __init__(self, package_reference: _Optional[_Union[_package_reference_pb2.PackageReference, _Mapping]] = ..., synchronizer_id: _Optional[str] = ...) -> None: ...

class PartySignatures(_message.Message):
    __slots__ = ["signatures"]
    SIGNATURES_FIELD_NUMBER: _ClassVar[int]
    signatures: _containers.RepeatedCompositeFieldContainer[SinglePartySignatures]
    def __init__(self, signatures: _Optional[_Iterable[_Union[SinglePartySignatures, _Mapping]]] = ...) -> None: ...

class PrepareSubmissionRequest(_message.Message):
    __slots__ = ["act_as", "command_id", "commands", "disclosed_contracts", "min_ledger_time", "package_id_selection_preference", "prefetch_contract_keys", "read_as", "synchronizer_id", "user_id", "verbose_hashing"]
    ACT_AS_FIELD_NUMBER: _ClassVar[int]
    COMMANDS_FIELD_NUMBER: _ClassVar[int]
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    DISCLOSED_CONTRACTS_FIELD_NUMBER: _ClassVar[int]
    MIN_LEDGER_TIME_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_ID_SELECTION_PREFERENCE_FIELD_NUMBER: _ClassVar[int]
    PREFETCH_CONTRACT_KEYS_FIELD_NUMBER: _ClassVar[int]
    READ_AS_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    USER_ID_FIELD_NUMBER: _ClassVar[int]
    VERBOSE_HASHING_FIELD_NUMBER: _ClassVar[int]
    act_as: _containers.RepeatedScalarFieldContainer[str]
    command_id: str
    commands: _containers.RepeatedCompositeFieldContainer[_commands_pb2.Command]
    disclosed_contracts: _containers.RepeatedCompositeFieldContainer[_commands_pb2.DisclosedContract]
    min_ledger_time: MinLedgerTime
    package_id_selection_preference: _containers.RepeatedScalarFieldContainer[str]
    prefetch_contract_keys: _containers.RepeatedCompositeFieldContainer[_commands_pb2.PrefetchContractKey]
    read_as: _containers.RepeatedScalarFieldContainer[str]
    synchronizer_id: str
    user_id: str
    verbose_hashing: bool
    def __init__(self, user_id: _Optional[str] = ..., command_id: _Optional[str] = ..., commands: _Optional[_Iterable[_Union[_commands_pb2.Command, _Mapping]]] = ..., min_ledger_time: _Optional[_Union[MinLedgerTime, _Mapping]] = ..., act_as: _Optional[_Iterable[str]] = ..., read_as: _Optional[_Iterable[str]] = ..., disclosed_contracts: _Optional[_Iterable[_Union[_commands_pb2.DisclosedContract, _Mapping]]] = ..., synchronizer_id: _Optional[str] = ..., package_id_selection_preference: _Optional[_Iterable[str]] = ..., verbose_hashing: bool = ..., prefetch_contract_keys: _Optional[_Iterable[_Union[_commands_pb2.PrefetchContractKey, _Mapping]]] = ...) -> None: ...

class PrepareSubmissionResponse(_message.Message):
    __slots__ = ["hashing_details", "hashing_scheme_version", "prepared_transaction", "prepared_transaction_hash"]
    HASHING_DETAILS_FIELD_NUMBER: _ClassVar[int]
    HASHING_SCHEME_VERSION_FIELD_NUMBER: _ClassVar[int]
    PREPARED_TRANSACTION_FIELD_NUMBER: _ClassVar[int]
    PREPARED_TRANSACTION_HASH_FIELD_NUMBER: _ClassVar[int]
    hashing_details: str
    hashing_scheme_version: HashingSchemeVersion
    prepared_transaction: PreparedTransaction
    prepared_transaction_hash: bytes
    def __init__(self, prepared_transaction: _Optional[_Union[PreparedTransaction, _Mapping]] = ..., prepared_transaction_hash: _Optional[bytes] = ..., hashing_scheme_version: _Optional[_Union[HashingSchemeVersion, str]] = ..., hashing_details: _Optional[str] = ...) -> None: ...

class PreparedTransaction(_message.Message):
    __slots__ = ["metadata", "transaction"]
    METADATA_FIELD_NUMBER: _ClassVar[int]
    TRANSACTION_FIELD_NUMBER: _ClassVar[int]
    metadata: Metadata
    transaction: DamlTransaction
    def __init__(self, transaction: _Optional[_Union[DamlTransaction, _Mapping]] = ..., metadata: _Optional[_Union[Metadata, _Mapping]] = ...) -> None: ...

class Signature(_message.Message):
    __slots__ = ["format", "signature", "signed_by", "signing_algorithm_spec"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    SIGNATURE_FIELD_NUMBER: _ClassVar[int]
    SIGNED_BY_FIELD_NUMBER: _ClassVar[int]
    SIGNING_ALGORITHM_SPEC_FIELD_NUMBER: _ClassVar[int]
    format: SignatureFormat
    signature: bytes
    signed_by: str
    signing_algorithm_spec: SigningAlgorithmSpec
    def __init__(self, format: _Optional[_Union[SignatureFormat, str]] = ..., signature: _Optional[bytes] = ..., signed_by: _Optional[str] = ..., signing_algorithm_spec: _Optional[_Union[SigningAlgorithmSpec, str]] = ...) -> None: ...

class SinglePartySignatures(_message.Message):
    __slots__ = ["party", "signatures"]
    PARTY_FIELD_NUMBER: _ClassVar[int]
    SIGNATURES_FIELD_NUMBER: _ClassVar[int]
    party: str
    signatures: _containers.RepeatedCompositeFieldContainer[Signature]
    def __init__(self, party: _Optional[str] = ..., signatures: _Optional[_Iterable[_Union[Signature, _Mapping]]] = ...) -> None: ...

class HashingSchemeVersion(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SigningAlgorithmSpec(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SignatureFormat(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
