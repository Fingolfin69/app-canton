from com.daml.ledger.api.v2 import offset_checkpoint_pb2 as _offset_checkpoint_pb2
from com.daml.ledger.api.v2 import trace_context_pb2 as _trace_context_pb2
from google.protobuf import duration_pb2 as _duration_pb2
from google.rpc import status_pb2 as _status_pb2
from google.protobuf.internal import containers as _containers
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor

class Completion(_message.Message):
    __slots__ = ["act_as", "command_id", "deduplication_duration", "deduplication_offset", "offset", "status", "submission_id", "synchronizer_time", "trace_context", "update_id", "user_id"]
    ACT_AS_FIELD_NUMBER: _ClassVar[int]
    COMMAND_ID_FIELD_NUMBER: _ClassVar[int]
    DEDUPLICATION_DURATION_FIELD_NUMBER: _ClassVar[int]
    DEDUPLICATION_OFFSET_FIELD_NUMBER: _ClassVar[int]
    OFFSET_FIELD_NUMBER: _ClassVar[int]
    STATUS_FIELD_NUMBER: _ClassVar[int]
    SUBMISSION_ID_FIELD_NUMBER: _ClassVar[int]
    SYNCHRONIZER_TIME_FIELD_NUMBER: _ClassVar[int]
    TRACE_CONTEXT_FIELD_NUMBER: _ClassVar[int]
    UPDATE_ID_FIELD_NUMBER: _ClassVar[int]
    USER_ID_FIELD_NUMBER: _ClassVar[int]
    act_as: _containers.RepeatedScalarFieldContainer[str]
    command_id: str
    deduplication_duration: _duration_pb2.Duration
    deduplication_offset: int
    offset: int
    status: _status_pb2.Status
    submission_id: str
    synchronizer_time: _offset_checkpoint_pb2.SynchronizerTime
    trace_context: _trace_context_pb2.TraceContext
    update_id: str
    user_id: str
    def __init__(self, command_id: _Optional[str] = ..., status: _Optional[_Union[_status_pb2.Status, _Mapping]] = ..., update_id: _Optional[str] = ..., user_id: _Optional[str] = ..., act_as: _Optional[_Iterable[str]] = ..., submission_id: _Optional[str] = ..., deduplication_offset: _Optional[int] = ..., deduplication_duration: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., trace_context: _Optional[_Union[_trace_context_pb2.TraceContext, _Mapping]] = ..., offset: _Optional[int] = ..., synchronizer_time: _Optional[_Union[_offset_checkpoint_pb2.SynchronizerTime, _Mapping]] = ...) -> None: ...
