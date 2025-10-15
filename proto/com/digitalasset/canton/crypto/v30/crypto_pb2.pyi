from google.protobuf.internal import containers as _containers
from google.protobuf.internal import enum_type_wrapper as _enum_type_wrapper
from google.protobuf import descriptor as _descriptor
from google.protobuf import message as _message
from typing import ClassVar as _ClassVar, Iterable as _Iterable, Mapping as _Mapping, Optional as _Optional, Union as _Union

CRYPTO_KEY_FORMAT_DER: CryptoKeyFormat
CRYPTO_KEY_FORMAT_DER_PKCS8_PRIVATE_KEY_INFO: CryptoKeyFormat
CRYPTO_KEY_FORMAT_DER_X509_SUBJECT_PUBLIC_KEY_INFO: CryptoKeyFormat
CRYPTO_KEY_FORMAT_RAW: CryptoKeyFormat
CRYPTO_KEY_FORMAT_SYMBOLIC: CryptoKeyFormat
CRYPTO_KEY_FORMAT_UNSPECIFIED: CryptoKeyFormat
DESCRIPTOR: _descriptor.FileDescriptor
ENCRYPTION_ALGORITHM_SPEC_ECIES_HKDF_HMAC_SHA256_AES128CBC: EncryptionAlgorithmSpec
ENCRYPTION_ALGORITHM_SPEC_ECIES_HKDF_HMAC_SHA256_AES128GCM: EncryptionAlgorithmSpec
ENCRYPTION_ALGORITHM_SPEC_RSA_OAEP_SHA256: EncryptionAlgorithmSpec
ENCRYPTION_ALGORITHM_SPEC_UNSPECIFIED: EncryptionAlgorithmSpec
ENCRYPTION_KEY_SCHEME_ECIES_P256_HKDF_HMAC_SHA256_AES128GCM: EncryptionKeyScheme
ENCRYPTION_KEY_SCHEME_ECIES_P256_HMAC_SHA256A_ES128CBC: EncryptionKeyScheme
ENCRYPTION_KEY_SCHEME_RSA2048_OAEP_SHA256: EncryptionKeyScheme
ENCRYPTION_KEY_SCHEME_UNSPECIFIED: EncryptionKeyScheme
ENCRYPTION_KEY_SPEC_EC_P256: EncryptionKeySpec
ENCRYPTION_KEY_SPEC_RSA_2048: EncryptionKeySpec
ENCRYPTION_KEY_SPEC_UNSPECIFIED: EncryptionKeySpec
HASH_ALGORITHM_SHA256: HashAlgorithm
HASH_ALGORITHM_UNSPECIFIED: HashAlgorithm
HMAC_ALGORITHM_HMAC_SHA256: HmacAlgorithm
HMAC_ALGORITHM_UNSPECIFIED: HmacAlgorithm
KEY_PURPOSE_ENCRYPTION: KeyPurpose
KEY_PURPOSE_SIGNING: KeyPurpose
KEY_PURPOSE_UNSPECIFIED: KeyPurpose
PBKDF_SCHEME_ARGON2ID_MODE1: PbkdfScheme
PBKDF_SCHEME_UNSPECIFIED: PbkdfScheme
SIGNATURE_FORMAT_CONCAT: SignatureFormat
SIGNATURE_FORMAT_DER: SignatureFormat
SIGNATURE_FORMAT_RAW: SignatureFormat
SIGNATURE_FORMAT_SYMBOLIC: SignatureFormat
SIGNATURE_FORMAT_UNSPECIFIED: SignatureFormat
SIGNING_ALGORITHM_SPEC_EC_DSA_SHA_256: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_EC_DSA_SHA_384: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_ED25519: SigningAlgorithmSpec
SIGNING_ALGORITHM_SPEC_UNSPECIFIED: SigningAlgorithmSpec
SIGNING_KEY_SCHEME_EC_DSA_P256: SigningKeyScheme
SIGNING_KEY_SCHEME_EC_DSA_P384: SigningKeyScheme
SIGNING_KEY_SCHEME_ED25519: SigningKeyScheme
SIGNING_KEY_SCHEME_UNSPECIFIED: SigningKeyScheme
SIGNING_KEY_SPEC_EC_CURVE25519: SigningKeySpec
SIGNING_KEY_SPEC_EC_P256: SigningKeySpec
SIGNING_KEY_SPEC_EC_P384: SigningKeySpec
SIGNING_KEY_SPEC_EC_SECP256K1: SigningKeySpec
SIGNING_KEY_SPEC_UNSPECIFIED: SigningKeySpec
SIGNING_KEY_USAGE_IDENTITY_DELEGATION: SigningKeyUsage
SIGNING_KEY_USAGE_NAMESPACE: SigningKeyUsage
SIGNING_KEY_USAGE_PROOF_OF_OWNERSHIP: SigningKeyUsage
SIGNING_KEY_USAGE_PROTOCOL: SigningKeyUsage
SIGNING_KEY_USAGE_SEQUENCER_AUTHENTICATION: SigningKeyUsage
SIGNING_KEY_USAGE_UNSPECIFIED: SigningKeyUsage
SYMMETRIC_KEY_SCHEME_AES128GCM: SymmetricKeyScheme
SYMMETRIC_KEY_SCHEME_UNSPECIFIED: SymmetricKeyScheme

class AsymmetricEncrypted(_message.Message):
    __slots__ = ["ciphertext", "encryption_algorithm_spec", "fingerprint"]
    CIPHERTEXT_FIELD_NUMBER: _ClassVar[int]
    ENCRYPTION_ALGORITHM_SPEC_FIELD_NUMBER: _ClassVar[int]
    FINGERPRINT_FIELD_NUMBER: _ClassVar[int]
    ciphertext: bytes
    encryption_algorithm_spec: EncryptionAlgorithmSpec
    fingerprint: str
    def __init__(self, ciphertext: _Optional[bytes] = ..., encryption_algorithm_spec: _Optional[_Union[EncryptionAlgorithmSpec, str]] = ..., fingerprint: _Optional[str] = ...) -> None: ...

class CryptoKeyPair(_message.Message):
    __slots__ = ["encryption_key_pair", "signing_key_pair"]
    ENCRYPTION_KEY_PAIR_FIELD_NUMBER: _ClassVar[int]
    SIGNING_KEY_PAIR_FIELD_NUMBER: _ClassVar[int]
    encryption_key_pair: EncryptionKeyPair
    signing_key_pair: SigningKeyPair
    def __init__(self, signing_key_pair: _Optional[_Union[SigningKeyPair, _Mapping]] = ..., encryption_key_pair: _Optional[_Union[EncryptionKeyPair, _Mapping]] = ...) -> None: ...

class EncryptionKeyPair(_message.Message):
    __slots__ = ["private_key", "public_key"]
    PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    private_key: EncryptionPrivateKey
    public_key: EncryptionPublicKey
    def __init__(self, public_key: _Optional[_Union[EncryptionPublicKey, _Mapping]] = ..., private_key: _Optional[_Union[EncryptionPrivateKey, _Mapping]] = ...) -> None: ...

class EncryptionPrivateKey(_message.Message):
    __slots__ = ["format", "id", "key_spec", "private_key", "scheme"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    ID_FIELD_NUMBER: _ClassVar[int]
    KEY_SPEC_FIELD_NUMBER: _ClassVar[int]
    PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    SCHEME_FIELD_NUMBER: _ClassVar[int]
    format: CryptoKeyFormat
    id: str
    key_spec: EncryptionKeySpec
    private_key: bytes
    scheme: EncryptionKeyScheme
    def __init__(self, id: _Optional[str] = ..., format: _Optional[_Union[CryptoKeyFormat, str]] = ..., private_key: _Optional[bytes] = ..., scheme: _Optional[_Union[EncryptionKeyScheme, str]] = ..., key_spec: _Optional[_Union[EncryptionKeySpec, str]] = ...) -> None: ...

class EncryptionPublicKey(_message.Message):
    __slots__ = ["format", "key_spec", "public_key", "scheme"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    KEY_SPEC_FIELD_NUMBER: _ClassVar[int]
    PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    SCHEME_FIELD_NUMBER: _ClassVar[int]
    format: CryptoKeyFormat
    key_spec: EncryptionKeySpec
    public_key: bytes
    scheme: EncryptionKeyScheme
    def __init__(self, format: _Optional[_Union[CryptoKeyFormat, str]] = ..., public_key: _Optional[bytes] = ..., scheme: _Optional[_Union[EncryptionKeyScheme, str]] = ..., key_spec: _Optional[_Union[EncryptionKeySpec, str]] = ...) -> None: ...

class Hmac(_message.Message):
    __slots__ = ["algorithm", "hmac"]
    ALGORITHM_FIELD_NUMBER: _ClassVar[int]
    HMAC_FIELD_NUMBER: _ClassVar[int]
    algorithm: HmacAlgorithm
    hmac: bytes
    def __init__(self, algorithm: _Optional[_Union[HmacAlgorithm, str]] = ..., hmac: _Optional[bytes] = ...) -> None: ...

class PasswordBasedEncrypted(_message.Message):
    __slots__ = ["ciphertext", "pbkdf_scheme", "salt", "symmetric_key_scheme"]
    CIPHERTEXT_FIELD_NUMBER: _ClassVar[int]
    PBKDF_SCHEME_FIELD_NUMBER: _ClassVar[int]
    SALT_FIELD_NUMBER: _ClassVar[int]
    SYMMETRIC_KEY_SCHEME_FIELD_NUMBER: _ClassVar[int]
    ciphertext: bytes
    pbkdf_scheme: PbkdfScheme
    salt: bytes
    symmetric_key_scheme: SymmetricKeyScheme
    def __init__(self, ciphertext: _Optional[bytes] = ..., symmetric_key_scheme: _Optional[_Union[SymmetricKeyScheme, str]] = ..., pbkdf_scheme: _Optional[_Union[PbkdfScheme, str]] = ..., salt: _Optional[bytes] = ...) -> None: ...

class PrivateKey(_message.Message):
    __slots__ = ["encryption_private_key", "signing_private_key"]
    ENCRYPTION_PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    SIGNING_PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    encryption_private_key: EncryptionPrivateKey
    signing_private_key: SigningPrivateKey
    def __init__(self, signing_private_key: _Optional[_Union[SigningPrivateKey, _Mapping]] = ..., encryption_private_key: _Optional[_Union[EncryptionPrivateKey, _Mapping]] = ...) -> None: ...

class PublicKey(_message.Message):
    __slots__ = ["encryption_public_key", "signing_public_key"]
    ENCRYPTION_PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    SIGNING_PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    encryption_public_key: EncryptionPublicKey
    signing_public_key: SigningPublicKey
    def __init__(self, signing_public_key: _Optional[_Union[SigningPublicKey, _Mapping]] = ..., encryption_public_key: _Optional[_Union[EncryptionPublicKey, _Mapping]] = ...) -> None: ...

class PublicKeyWithName(_message.Message):
    __slots__ = ["name", "public_key"]
    NAME_FIELD_NUMBER: _ClassVar[int]
    PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    name: str
    public_key: PublicKey
    def __init__(self, public_key: _Optional[_Union[PublicKey, _Mapping]] = ..., name: _Optional[str] = ...) -> None: ...

class RequiredEncryptionSpecs(_message.Message):
    __slots__ = ["algorithms", "keys"]
    ALGORITHMS_FIELD_NUMBER: _ClassVar[int]
    KEYS_FIELD_NUMBER: _ClassVar[int]
    algorithms: _containers.RepeatedScalarFieldContainer[EncryptionAlgorithmSpec]
    keys: _containers.RepeatedScalarFieldContainer[EncryptionKeySpec]
    def __init__(self, algorithms: _Optional[_Iterable[_Union[EncryptionAlgorithmSpec, str]]] = ..., keys: _Optional[_Iterable[_Union[EncryptionKeySpec, str]]] = ...) -> None: ...

class RequiredSigningSpecs(_message.Message):
    __slots__ = ["algorithms", "keys"]
    ALGORITHMS_FIELD_NUMBER: _ClassVar[int]
    KEYS_FIELD_NUMBER: _ClassVar[int]
    algorithms: _containers.RepeatedScalarFieldContainer[SigningAlgorithmSpec]
    keys: _containers.RepeatedScalarFieldContainer[SigningKeySpec]
    def __init__(self, algorithms: _Optional[_Iterable[_Union[SigningAlgorithmSpec, str]]] = ..., keys: _Optional[_Iterable[_Union[SigningKeySpec, str]]] = ...) -> None: ...

class Salt(_message.Message):
    __slots__ = ["hmac", "salt"]
    HMAC_FIELD_NUMBER: _ClassVar[int]
    SALT_FIELD_NUMBER: _ClassVar[int]
    hmac: HmacAlgorithm
    salt: bytes
    def __init__(self, hmac: _Optional[_Union[HmacAlgorithm, str]] = ..., salt: _Optional[bytes] = ...) -> None: ...

class Signature(_message.Message):
    __slots__ = ["format", "signature", "signature_delegation", "signed_by", "signing_algorithm_spec"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    SIGNATURE_DELEGATION_FIELD_NUMBER: _ClassVar[int]
    SIGNATURE_FIELD_NUMBER: _ClassVar[int]
    SIGNED_BY_FIELD_NUMBER: _ClassVar[int]
    SIGNING_ALGORITHM_SPEC_FIELD_NUMBER: _ClassVar[int]
    format: SignatureFormat
    signature: bytes
    signature_delegation: SignatureDelegation
    signed_by: str
    signing_algorithm_spec: SigningAlgorithmSpec
    def __init__(self, format: _Optional[_Union[SignatureFormat, str]] = ..., signature: _Optional[bytes] = ..., signed_by: _Optional[str] = ..., signing_algorithm_spec: _Optional[_Union[SigningAlgorithmSpec, str]] = ..., signature_delegation: _Optional[_Union[SignatureDelegation, _Mapping]] = ...) -> None: ...

class SignatureDelegation(_message.Message):
    __slots__ = ["format", "session_key", "session_key_spec", "signature", "signing_algorithm_spec", "validity_period_duration_seconds", "validity_period_from_inclusive"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    SESSION_KEY_FIELD_NUMBER: _ClassVar[int]
    SESSION_KEY_SPEC_FIELD_NUMBER: _ClassVar[int]
    SIGNATURE_FIELD_NUMBER: _ClassVar[int]
    SIGNING_ALGORITHM_SPEC_FIELD_NUMBER: _ClassVar[int]
    VALIDITY_PERIOD_DURATION_SECONDS_FIELD_NUMBER: _ClassVar[int]
    VALIDITY_PERIOD_FROM_INCLUSIVE_FIELD_NUMBER: _ClassVar[int]
    format: SignatureFormat
    session_key: bytes
    session_key_spec: SigningKeySpec
    signature: bytes
    signing_algorithm_spec: SigningAlgorithmSpec
    validity_period_duration_seconds: int
    validity_period_from_inclusive: int
    def __init__(self, session_key: _Optional[bytes] = ..., session_key_spec: _Optional[_Union[SigningKeySpec, str]] = ..., validity_period_from_inclusive: _Optional[int] = ..., validity_period_duration_seconds: _Optional[int] = ..., format: _Optional[_Union[SignatureFormat, str]] = ..., signature: _Optional[bytes] = ..., signing_algorithm_spec: _Optional[_Union[SigningAlgorithmSpec, str]] = ...) -> None: ...

class SigningKeyPair(_message.Message):
    __slots__ = ["private_key", "public_key"]
    PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    private_key: SigningPrivateKey
    public_key: SigningPublicKey
    def __init__(self, public_key: _Optional[_Union[SigningPublicKey, _Mapping]] = ..., private_key: _Optional[_Union[SigningPrivateKey, _Mapping]] = ...) -> None: ...

class SigningPrivateKey(_message.Message):
    __slots__ = ["format", "id", "key_spec", "private_key", "scheme", "usage"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    ID_FIELD_NUMBER: _ClassVar[int]
    KEY_SPEC_FIELD_NUMBER: _ClassVar[int]
    PRIVATE_KEY_FIELD_NUMBER: _ClassVar[int]
    SCHEME_FIELD_NUMBER: _ClassVar[int]
    USAGE_FIELD_NUMBER: _ClassVar[int]
    format: CryptoKeyFormat
    id: str
    key_spec: SigningKeySpec
    private_key: bytes
    scheme: SigningKeyScheme
    usage: _containers.RepeatedScalarFieldContainer[SigningKeyUsage]
    def __init__(self, id: _Optional[str] = ..., format: _Optional[_Union[CryptoKeyFormat, str]] = ..., private_key: _Optional[bytes] = ..., scheme: _Optional[_Union[SigningKeyScheme, str]] = ..., usage: _Optional[_Iterable[_Union[SigningKeyUsage, str]]] = ..., key_spec: _Optional[_Union[SigningKeySpec, str]] = ...) -> None: ...

class SigningPublicKey(_message.Message):
    __slots__ = ["format", "key_spec", "public_key", "scheme", "usage"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    KEY_SPEC_FIELD_NUMBER: _ClassVar[int]
    PUBLIC_KEY_FIELD_NUMBER: _ClassVar[int]
    SCHEME_FIELD_NUMBER: _ClassVar[int]
    USAGE_FIELD_NUMBER: _ClassVar[int]
    format: CryptoKeyFormat
    key_spec: SigningKeySpec
    public_key: bytes
    scheme: SigningKeyScheme
    usage: _containers.RepeatedScalarFieldContainer[SigningKeyUsage]
    def __init__(self, format: _Optional[_Union[CryptoKeyFormat, str]] = ..., public_key: _Optional[bytes] = ..., scheme: _Optional[_Union[SigningKeyScheme, str]] = ..., usage: _Optional[_Iterable[_Union[SigningKeyUsage, str]]] = ..., key_spec: _Optional[_Union[SigningKeySpec, str]] = ...) -> None: ...

class SymmetricKey(_message.Message):
    __slots__ = ["format", "key", "scheme"]
    FORMAT_FIELD_NUMBER: _ClassVar[int]
    KEY_FIELD_NUMBER: _ClassVar[int]
    SCHEME_FIELD_NUMBER: _ClassVar[int]
    format: CryptoKeyFormat
    key: bytes
    scheme: SymmetricKeyScheme
    def __init__(self, format: _Optional[_Union[CryptoKeyFormat, str]] = ..., key: _Optional[bytes] = ..., scheme: _Optional[_Union[SymmetricKeyScheme, str]] = ...) -> None: ...

class HashAlgorithm(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class HmacAlgorithm(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SignatureFormat(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class EncryptionKeySpec(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SigningKeySpec(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class KeyPurpose(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SigningKeyUsage(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SigningAlgorithmSpec(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SigningKeyScheme(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class EncryptionAlgorithmSpec(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class EncryptionKeyScheme(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class SymmetricKeyScheme(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class CryptoKeyFormat(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []

class PbkdfScheme(int, metaclass=_enum_type_wrapper.EnumTypeWrapper):
    __slots__ = []
