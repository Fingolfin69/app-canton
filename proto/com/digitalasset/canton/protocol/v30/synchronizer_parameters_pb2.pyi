from com.digitalasset.canton.protocol.v30 import traffic_control_parameters_pb2 as _traffic_control_parameters_pb2
from google.protobuf import duration_pb2 as _duration_pb2
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Mapping as _Mapping, Optional as _Optional, Union as _Union

DESCRIPTOR: _descriptor.FileDescriptor
ONBOARDING_RESTRICTION_RESTRICTED_LOCKED: OnboardingRestriction
ONBOARDING_RESTRICTION_RESTRICTED_OPEN: OnboardingRestriction
ONBOARDING_RESTRICTION_UNRESTRICTED_LOCKED: OnboardingRestriction
ONBOARDING_RESTRICTION_UNRESTRICTED_OPEN: OnboardingRestriction
ONBOARDING_RESTRICTION_UNSPECIFIED: OnboardingRestriction

class AcsCommitmentsCatchUpConfig(_message.Message):
    __slots__ = ["catchup_interval_skip", "nr_intervals_to_trigger_catchup"]
    CATCHUP_INTERVAL_SKIP_FIELD_NUMBER: _ClassVar[int]
    NR_INTERVALS_TO_TRIGGER_CATCHUP_FIELD_NUMBER: _ClassVar[int]
    catchup_interval_skip: int
    nr_intervals_to_trigger_catchup: int
    def __init__(self, catchup_interval_skip: _Optional[int] = ..., nr_intervals_to_trigger_catchup: _Optional[int] = ...) -> None: ...

class DynamicSynchronizerParameters(_message.Message):
    __slots__ = ["acs_commitments_catchup", "assignment_exclusivity_timeout", "confirmation_response_timeout", "ledger_time_record_time_tolerance", "max_request_size", "mediator_deduplication_timeout", "mediator_reaction_timeout", "onboarding_restriction", "participant_synchronizer_limits", "preparation_time_record_time_tolerance", "reconciliation_interval", "sequencer_aggregate_submission_timeout", "topology_change_delay", "traffic_control"]
    ACS_COMMITMENTS_CATCHUP_FIELD_NUMBER: _ClassVar[int]
    ASSIGNMENT_EXCLUSIVITY_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    CONFIRMATION_RESPONSE_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    LEDGER_TIME_RECORD_TIME_TOLERANCE_FIELD_NUMBER: _ClassVar[int]
    MAX_REQUEST_SIZE_FIELD_NUMBER: _ClassVar[int]
    MEDIATOR_DEDUPLICATION_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    MEDIATOR_REACTION_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    ONBOARDING_RESTRICTION_FIELD_NUMBER: _ClassVar[int]
    PARTICIPANT_SYNCHRONIZER_LIMITS_FIELD_NUMBER: _ClassVar[int]
    PREPARATION_TIME_RECORD_TIME_TOLERANCE_FIELD_NUMBER: _ClassVar[int]
    RECONCILIATION_INTERVAL_FIELD_NUMBER: _ClassVar[int]
    SEQUENCER_AGGREGATE_SUBMISSION_TIMEOUT_FIELD_NUMBER: _ClassVar[int]
    TOPOLOGY_CHANGE_DELAY_FIELD_NUMBER: _ClassVar[int]
    TRAFFIC_CONTROL_FIELD_NUMBER: _ClassVar[int]
    acs_commitments_catchup: AcsCommitmentsCatchUpConfig
    assignment_exclusivity_timeout: _duration_pb2.Duration
    confirmation_response_timeout: _duration_pb2.Duration
    ledger_time_record_time_tolerance: _duration_pb2.Duration
    max_request_size: int
    mediator_deduplication_timeout: _duration_pb2.Duration
    mediator_reaction_timeout: _duration_pb2.Duration
    onboarding_restriction: OnboardingRestriction
    participant_synchronizer_limits: ParticipantSynchronizerLimits
    preparation_time_record_time_tolerance: _duration_pb2.Duration
    reconciliation_interval: _duration_pb2.Duration
    sequencer_aggregate_submission_timeout: _duration_pb2.Duration
    topology_change_delay: _duration_pb2.Duration
    traffic_control: _traffic_control_parameters_pb2.TrafficControlParameters
    def __init__(self, confirmation_response_timeout: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., mediator_reaction_timeout: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., assignment_exclusivity_timeout: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., topology_change_delay: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., ledger_time_record_time_tolerance: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., reconciliation_interval: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., mediator_deduplication_timeout: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., max_request_size: _Optional[int] = ..., onboarding_restriction: _Optional[_Union[OnboardingRestriction, str]] = ..., participant_synchronizer_limits: _Optional[_Union[ParticipantSynchronizerLimits, _Mapping]] = ..., sequencer_aggregate_submission_timeout: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ..., traffic_control: _Optional[_Union[_traffic_control_parameters_pb2.TrafficControlParameters, _Mapping]] = ..., acs_commitments_catchup: _Optional[_Union[AcsCommitmentsCatchUpConfig, _Mapping]] = ..., preparation_time_record_time_tolerance: _Optional[_Union[_duration_pb2.Duration, _Mapping]] = ...) -> None: ...

class ParticipantSynchronizerLimits(_message.Message):
    __slots__ = ["confirmation_requests_max_rate"]
    CONFIRMATION_REQUESTS_MAX_RATE_FIELD_NUMBER: _ClassVar[int]
    confirmation_requests_max_rate: int
    def __init__(self, confirmation_requests_max_rate: _Optional[int] = ...) -> None: ...

class OnboardingRestriction(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
