from google.protobuf import empty_pb2 as _empty_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class Enum(_message.Message):
    __slots__ = ["constructor", "enum_id"]
    CONSTRUCTOR_FIELD_NUMBER: _ClassVar[int]
    ENUM_ID_FIELD_NUMBER: _ClassVar[int]
    constructor: str
    enum_id: Identifier
    def __init__(self, enum_id: _Optional[_Union[Identifier, _Mapping]] = ..., constructor: _Optional[str] = ...) -> None: ...

class GenMap(_message.Message):
    __slots__ = ["entries"]
    class Entry(_message.Message):
        __slots__ = ["key", "value"]
        KEY_FIELD_NUMBER: _ClassVar[int]
        VALUE_FIELD_NUMBER: _ClassVar[int]
        key: Value
        value: Value
        def __init__(self, key: _Optional[_Union[Value, _Mapping]] = ..., value: _Optional[_Union[Value, _Mapping]] = ...) -> None: ...
    ENTRIES_FIELD_NUMBER: _ClassVar[int]
    entries: _containers.RepeatedCompositeFieldContainer[GenMap.Entry]
    def __init__(self, entries: _Optional[_Iterable[_Union[GenMap.Entry, _Mapping]]] = ...) -> None: ...

class Identifier(_message.Message):
    __slots__ = ["entity_name", "module_name", "package_id"]
    ENTITY_NAME_FIELD_NUMBER: _ClassVar[int]
    MODULE_NAME_FIELD_NUMBER: _ClassVar[int]
    PACKAGE_ID_FIELD_NUMBER: _ClassVar[int]
    entity_name: str
    module_name: str
    package_id: str
    def __init__(self, package_id: _Optional[str] = ..., module_name: _Optional[str] = ..., entity_name: _Optional[str] = ...) -> None: ...

class List(_message.Message):
    __slots__ = ["elements"]
    ELEMENTS_FIELD_NUMBER: _ClassVar[int]
    elements: _containers.RepeatedCompositeFieldContainer[Value]
    def __init__(self, elements: _Optional[_Iterable[_Union[Value, _Mapping]]] = ...) -> None: ...

class Optional(_message.Message):
    __slots__ = ["value"]
    VALUE_FIELD_NUMBER: _ClassVar[int]
    value: Value
    def __init__(self, value: _Optional[_Union[Value, _Mapping]] = ...) -> None: ...

class Record(_message.Message):
    __slots__ = ["fields", "record_id"]
    FIELDS_FIELD_NUMBER: _ClassVar[int]
    RECORD_ID_FIELD_NUMBER: _ClassVar[int]
    fields: _containers.RepeatedCompositeFieldContainer[RecordField]
    record_id: Identifier
    def __init__(self, record_id: _Optional[_Union[Identifier, _Mapping]] = ..., fields: _Optional[_Iterable[_Union[RecordField, _Mapping]]] = ...) -> None: ...

class RecordField(_message.Message):
    __slots__ = ["label", "value"]
    LABEL_FIELD_NUMBER: _ClassVar[int]
    VALUE_FIELD_NUMBER: _ClassVar[int]
    label: str
    value: Value
    def __init__(self, label: _Optional[str] = ..., value: _Optional[_Union[Value, _Mapping]] = ...) -> None: ...

class TextMap(_message.Message):
    __slots__ = ["entries"]
    class Entry(_message.Message):
        __slots__ = ["key", "value"]
        KEY_FIELD_NUMBER: _ClassVar[int]
        VALUE_FIELD_NUMBER: _ClassVar[int]
        key: str
        value: Value
        def __init__(self, key: _Optional[str] = ..., value: _Optional[_Union[Value, _Mapping]] = ...) -> None: ...
    ENTRIES_FIELD_NUMBER: _ClassVar[int]
    entries: _containers.RepeatedCompositeFieldContainer[TextMap.Entry]
    def __init__(self, entries: _Optional[_Iterable[_Union[TextMap.Entry, _Mapping]]] = ...) -> None: ...

class Value(_message.Message):
    __slots__ = ["bool_", "contract_id", "date", "enum_", "gen_map", "int64", "list", "numeric", "optional", "party", "record", "text", "text_map", "timestamp", "unit", "variant"]
    BOOL__FIELD_NUMBER: _ClassVar[int]
    CONTRACT_ID_FIELD_NUMBER: _ClassVar[int]
    DATE_FIELD_NUMBER: _ClassVar[int]
    ENUM__FIELD_NUMBER: _ClassVar[int]
    GEN_MAP_FIELD_NUMBER: _ClassVar[int]
    INT64_FIELD_NUMBER: _ClassVar[int]
    LIST_FIELD_NUMBER: _ClassVar[int]
    NUMERIC_FIELD_NUMBER: _ClassVar[int]
    OPTIONAL_FIELD_NUMBER: _ClassVar[int]
    PARTY_FIELD_NUMBER: _ClassVar[int]
    RECORD_FIELD_NUMBER: _ClassVar[int]
    TEXT_FIELD_NUMBER: _ClassVar[int]
    TEXT_MAP_FIELD_NUMBER: _ClassVar[int]
    TIMESTAMP_FIELD_NUMBER: _ClassVar[int]
    UNIT_FIELD_NUMBER: _ClassVar[int]
    VARIANT_FIELD_NUMBER: _ClassVar[int]
    bool_: bool
    contract_id: str
    date: int
    enum_: Enum
    gen_map: GenMap
    int64: int
    list: List
    numeric: str
    optional: Optional
    party: str
    record: Record
    text: str
    text_map: TextMap
    timestamp: int
    unit: _empty_pb2.Empty
    variant: Variant
    def __init__(self, unit: _Optional[_Union[_empty_pb2.Empty, _Mapping]] = ..., bool_: bool = ..., int64: _Optional[int] = ..., date: _Optional[int] = ..., timestamp: _Optional[int] = ..., numeric: _Optional[str] = ..., party: _Optional[str] = ..., text: _Optional[str] = ..., contract_id: _Optional[str] = ..., optional: _Optional[_Union[Optional, _Mapping]] = ..., list: _Optional[_Union[List, _Mapping]] = ..., text_map: _Optional[_Union[TextMap, _Mapping]] = ..., gen_map: _Optional[_Union[GenMap, _Mapping]] = ..., record: _Optional[_Union[Record, _Mapping]] = ..., variant: _Optional[_Union[Variant, _Mapping]] = ..., enum_: _Optional[_Union[Enum, _Mapping]] = ...) -> None: ...

class Variant(_message.Message):
    __slots__ = ["constructor", "value", "variant_id"]
    CONSTRUCTOR_FIELD_NUMBER: _ClassVar[int]
    VALUE_FIELD_NUMBER: _ClassVar[int]
    VARIANT_ID_FIELD_NUMBER: _ClassVar[int]
    constructor: str
    value: Value
    variant_id: Identifier
    def __init__(self, variant_id: _Optional[_Union[Identifier, _Mapping]] = ..., constructor: _Optional[str] = ..., value: _Optional[_Union[Value, _Mapping]] = ...) -> None: ...
