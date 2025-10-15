from com.daml.ledger.api.v2 import event_pb2 as _event_pb2
from com.daml.ledger.api.v2 import trace_context_pb2 as _trace_context_pb2
from google.protobuf import timestamp_pb2 as _timestamp_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class Transaction(_message.Message):
    __slots__ = ["command_id", "effective_at", "events", "offset", "record_time", "synchronizer_id", "trace_context", "update_id", "workflow_id"]
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    EFFECTIVE_AT_FIELD_NUMBER: _ClassVar[int]
    EVENTS_FIELD_NUMBER: _ClassVar[int]
    OFFSET_FIELD_NUMBER: _ClassVar[int]
    RECORD_TIME_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    TRACE_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    UPDATE_ID_FIELD_NUMBER: _ClassVar[int]
    WORKFLOW_ID_FIELD_NUMBER: _ClassVar[int]
    command_id: str
    effective_at: _timestamp_pb2.Timestamp
    events: _containers.RepeatedCompositeFieldContainer[_event_pb2.Event]
    offset: int
    record_time: _timestamp_pb2.Timestamp
    synchronizer_id: str
    trace_context: _trace_context_pb2.TraceContext
    update_id: str
    workflow_id: str
    def __init__(self, update_id: _Optional[str] = ..., command_id: _Optional[str] = ..., workflow_id: _Optional[str] = ..., effective_at: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ..., events: _Optional[_Iterable[_Union[_event_pb2.Event, _Mapping]]] = ..., offset: _Optional[int] = ..., synchronizer_id: _Optional[str] = ..., trace_context: _Optional[_Union[_trace_context_pb2.TraceContext, _Mapping]] = ..., record_time: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ...) -> None: ...

class TransactionTree(_message.Message):
    __slots__ = ["command_id", "effective_at", "events_by_id", "offset", "record_time", "synchronizer_id", "trace_context", "update_id", "workflow_id"]
    class EventsByIdEntry(_message.Message):
        __slots__ = ["key", "value"]
        KEY_FIELD_NUMBER: _ClassVar[int]
        VALUE_FIELD_NUMBER: _ClassVar[int]
        key: int
        value: TreeEvent
        def __init__(self, key: _Optional[int] = ..., value: _Optional[_Union[TreeEvent, _Mapping]] = ...) -> None: ...
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    EFFECTIVE_AT_FIELD_NUMBER: _ClassVar[int]
    EVENTS_BY_ID_FIELD_NUMBER: _ClassVar[int]
    OFFSET_FIELD_NUMBER: _ClassVar[int]
    RECORD_TIME_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_ID_FIELD_NUMBER: _ClassVar[int]
    TRACE_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    UPDATE_ID_FIELD_NUMBER: _ClassVar[int]
    WORKFLOW_ID_FIELD_NUMBER: _ClassVar[int]
    command_id: str
    effective_at: _timestamp_pb2.Timestamp
    events_by_id: _containers.MessageMap[int, TreeEvent]
    offset: int
    record_time: _timestamp_pb2.Timestamp
    synchronizer_id: str
    trace_context: _trace_context_pb2.TraceContext
    update_id: str
    workflow_id: str
    def __init__(self, update_id: _Optional[str] = ..., command_id: _Optional[str] = ..., workflow_id: _Optional[str] = ..., effective_at: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ..., offset: _Optional[int] = ..., events_by_id: _Optional[_Mapping[int, TreeEvent]] = ..., synchronizer_id: _Optional[str] = ..., trace_context: _Optional[_Union[_trace_context_pb2.TraceContext, _Mapping]] = ..., record_time: _Optional[_Union[_timestamp_pb2.Timestamp, _Mapping]] = ...) -> None: ...

class TreeEvent(_message.Message):
    __slots__ = ["created", "exercised"]
    CREATED_FIELD_NUMBER: _ClassVar[int]
    EXERCISED_FIELD_NUMBER: _ClassVar[int]
    created: _event_pb2.CreatedEvent
    exercised: _event_pb2.ExercisedEvent
    def __init__(self, created: _Optional[_Union[_event_pb2.CreatedEvent, _Mapping]] = ..., exercised: _Optional[_Union[_event_pb2.ExercisedEvent, _Mapping]] = ...) -> None: ...
