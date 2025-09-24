from com.daml.ledger.api.v2 import value_cb_pb2 as _value_cb_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class Create(_message.Message):
    __slots__ = ["argument", "contract_id", "lf_version", "package_name", "signatories", "stakeholders", "template_id"]
    ARGUMENT_FIELD_NUMBER: _ClassVar[int]
    CONTRACT_ID_FIELD_NUMBER: _ClassVar[int]
    LF_VERSION_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_NAME_FIELD_NUMBER: _ClassVar[int]
    SIGNATORIES_FIELD_NUMBER: _ClassVar[int]
    STAKEHOLDERS_FIELD_NUMBER: _ClassVar[int]
    TEMPLATE_ID_FIELD_NUMBER: _ClassVar[int]
    argument: _value_cb_pb2.Value
    contract_id: str
    lf_version: str
    package_name: str
    signatories: _containers.RepeatedScalarFieldContainer[str]
    stakeholders: _containers.RepeatedScalarFieldContainer[str]
    template_id: _value_cb_pb2.Identifier
    def __init__(self, lf_version: _Optional[str] = ..., contract_id: _Optional[str] = ..., package_name: _Optional[str] = ..., template_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ..., argument: _Optional[_Union[_value_cb_pb2.Value, _Mapping]] = ..., signatories: _Optional[_Iterable[str]] = ..., stakeholders: _Optional[_Iterable[str]] = ...) -> None: ...

class CreateNoArg(_message.Message):
    __slots__ = ["argument", "contract_id", "lf_version", "package_name", "signatories", "stakeholders", "template_id"]
    ARGUMENT_FIELD_NUMBER: _ClassVar[int]
    CONTRACT_ID_FIELD_NUMBER: _ClassVar[int]
    LF_VERSION_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_NAME_FIELD_NUMBER: _ClassVar[int]
    SIGNATORIES_FIELD_NUMBER: _ClassVar[int]
    STAKEHOLDERS_FIELD_NUMBER: _ClassVar[int]
    TEMPLATE_ID_FIELD_NUMBER: _ClassVar[int]
    argument: _value_cb_pb2.Value
    contract_id: str
    lf_version: str
    package_name: str
    signatories: _containers.RepeatedScalarFieldContainer[str]
    stakeholders: _containers.RepeatedScalarFieldContainer[str]
    template_id: _value_cb_pb2.Identifier
    def __init__(self, lf_version: _Optional[str] = ..., contract_id: _Optional[str] = ..., package_name: _Optional[str] = ..., template_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ..., argument: _Optional[_Union[_value_cb_pb2.Value, _Mapping]] = ..., signatories: _Optional[_Iterable[str]] = ..., stakeholders: _Optional[_Iterable[str]] = ...) -> None: ...

class Exercise(_message.Message):
    __slots__ = ["acting_parties", "children", "choice_id", "choice_observers", "chosen_value", "consuming", "contract_id", "exercise_result", "interface_id", "lf_version", "package_name", "signatories", "stakeholders", "template_id"]
    ACTING_PARTIES_FIELD_NUMBER: _ClassVar[int]
    CHILDREN_FIELD_NUMBER: _ClassVar[int]
    CHOICE_ID_FIELD_NUMBER: _ClassVar[int]
    CHOICE_OBSERVERS_FIELD_NUMBER: _ClassVar[int]
    CHOSEN_VALUE_FIELD_NUMBER: _ClassVar[int]
    CONSUMING_FIELD_NUMBER: _ClassVar[int]
    CONTRACT_ID_FIELD_NUMBER: _ClassVar[int]
    EXERCISE_RESULT_FIELD_NUMBER: _ClassVar[int]
    INTERFACE_ID_FIELD_NUMBER: _ClassVar[int]
    LF_VERSION_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_NAME_FIELD_NUMBER: _ClassVar[int]
    SIGNATORIES_FIELD_NUMBER: _ClassVar[int]
    STAKEHOLDERS_FIELD_NUMBER: _ClassVar[int]
    TEMPLATE_ID_FIELD_NUMBER: _ClassVar[int]
    acting_parties: _containers.RepeatedScalarFieldContainer[str]
    children: _containers.RepeatedScalarFieldContainer[str]
    choice_id: str
    choice_observers: _containers.RepeatedScalarFieldContainer[str]
    chosen_value: _value_cb_pb2.Value
    consuming: bool
    contract_id: str
    exercise_result: _value_cb_pb2.Value
    interface_id: _value_cb_pb2.Identifier
    lf_version: str
    package_name: str
    signatories: _containers.RepeatedScalarFieldContainer[str]
    stakeholders: _containers.RepeatedScalarFieldContainer[str]
    template_id: _value_cb_pb2.Identifier
    def __init__(self, lf_version: _Optional[str] = ..., contract_id: _Optional[str] = ..., package_name: _Optional[str] = ..., template_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ..., signatories: _Optional[_Iterable[str]] = ..., stakeholders: _Optional[_Iterable[str]] = ..., acting_parties: _Optional[_Iterable[str]] = ..., interface_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ..., choice_id: _Optional[str] = ..., chosen_value: _Optional[_Union[_value_cb_pb2.Value, _Mapping]] = ..., consuming: bool = ..., children: _Optional[_Iterable[str]] = ..., exercise_result: _Optional[_Union[_value_cb_pb2.Value, _Mapping]] = ..., choice_observers: _Optional[_Iterable[str]] = ...) -> None: ...

class Fetch(_message.Message):
    __slots__ = ["acting_parties", "contract_id", "interface_id", "lf_version", "package_name", "signatories", "stakeholders", "template_id"]
    ACTING_PARTIES_FIELD_NUMBER: _ClassVar[int]
    CONTRACT_ID_FIELD_NUMBER: _ClassVar[int]
    INTERFACE_ID_FIELD_NUMBER: _ClassVar[int]
    LF_VERSION_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_NAME_FIELD_NUMBER: _ClassVar[int]
    SIGNATORIES_FIELD_NUMBER: _ClassVar[int]
    STAKEHOLDERS_FIELD_NUMBER: _ClassVar[int]
    TEMPLATE_ID_FIELD_NUMBER: _ClassVar[int]
    acting_parties: _containers.RepeatedScalarFieldContainer[str]
    contract_id: str
    interface_id: _value_cb_pb2.Identifier
    lf_version: str
    package_name: str
    signatories: _containers.RepeatedScalarFieldContainer[str]
    stakeholders: _containers.RepeatedScalarFieldContainer[str]
    template_id: _value_cb_pb2.Identifier
    def __init__(self, lf_version: _Optional[str] = ..., contract_id: _Optional[str] = ..., package_name: _Optional[str] = ..., template_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ..., signatories: _Optional[_Iterable[str]] = ..., stakeholders: _Optional[_Iterable[str]] = ..., acting_parties: _Optional[_Iterable[str]] = ..., interface_id: _Optional[_Union[_value_cb_pb2.Identifier, _Mapping]] = ...) -> None: ...

class Node(_message.Message):
    __slots__ = ["create", "exercise", "fetch", "rollback"]
    CREATE_FIELD_NUMBER: _ClassVar[int]
    EXERCISE_FIELD_NUMBER: _ClassVar[int]
    FETCH_FIELD_NUMBER: _ClassVar[int]
    ROLLBACK_FIELD_NUMBER: _ClassVar[int]
    create: Create
    exercise: Exercise
    fetch: Fetch
    rollback: Rollback
    def __init__(self, create: _Optional[_Union[Create, _Mapping]] = ..., fetch: _Optional[_Union[Fetch, _Mapping]] = ..., exercise: _Optional[_Union[Exercise, _Mapping]] = ..., rollback: _Optional[_Union[Rollback, _Mapping]] = ...) -> None: ...

class Rollback(_message.Message):
    __slots__ = ["children"]
    CHILDREN_FIELD_NUMBER: _ClassVar[int]
    children: _containers.RepeatedScalarFieldContainer[str]
    def __init__(self, children: _Optional[_Iterable[str]] = ...) -> None: ...
