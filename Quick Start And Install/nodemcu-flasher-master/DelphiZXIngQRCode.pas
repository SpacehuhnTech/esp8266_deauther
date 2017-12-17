unit DelphiZXingQRCode;

// ZXing QRCode port to Delphi, by Debenu Pty Ltd (www.debenu.com)

// Original copyright notice
(*
 * Copyright 2008 ZXing authors
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *)

interface

type
  TQRCodeEncoding = (qrAuto, qrNumeric, qrAlphanumeric, qrISO88591, qrUTF8NoBOM, qrUTF8BOM);
  T2DBooleanArray = array of array of Boolean;

  TDelphiZXingQRCode = class
  protected
    FData: WideString;
    FRows: Integer;
    FColumns: Integer;
    FEncoding: TQRCodeEncoding;
    FQuietZone: Integer;
    FElements: T2DBooleanArray;
    procedure SetEncoding(NewEncoding: TQRCodeEncoding);
    procedure SetData(const NewData: WideString);
    procedure SetQuietZone(NewQuietZone: Integer);
    function GetIsBlack(Row, Column: Integer): Boolean;
    procedure Update;
  public
    constructor Create;
    property Data: WideString read FData write SetData;
    property Encoding: TQRCodeEncoding read FEncoding write SetEncoding;
    property QuietZone: Integer read FQuietZone write SetQuietZone;
    property Rows: Integer read FRows;
    property Columns: Integer read FColumns;
    property IsBlack[Row, Column: Integer]: Boolean read GetIsBlack;
  end;

implementation

uses
  contnrs, Math, Classes;

type
  TByteArray = array of Byte;
  T2DByteArray = array of array of Byte;
  TIntegerArray = array of Integer;

const
  NUM_MASK_PATTERNS = 8;

  QUIET_ZONE_SIZE = 4;

  ALPHANUMERIC_TABLE: array[0..95] of Integer = (
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x00-0x0f
      -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1, -1,  // 0x10-0x1f
      36, -1, -1, -1, 37, 38, -1, -1, -1, -1, 39, 40, -1, 41, 42, 43,  // 0x20-0x2f
      0,   1,  2,  3,  4,  5,  6,  7,  8,  9, 44, -1, -1, -1, -1, -1,  // 0x30-0x3f
      -1, 10, 11, 12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24,  // 0x40-0x4f
      25, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, -1, -1, -1, -1, -1   // 0x50-0x5f
  );

  DEFAULT_BYTE_MODE_ENCODING = 'ISO-8859-1';

  POSITION_DETECTION_PATTERN: array[0..6, 0..6] of Integer = (
    (1, 1, 1, 1, 1, 1, 1),
    (1, 0, 0, 0, 0, 0, 1),
    (1, 0, 1, 1, 1, 0, 1),
    (1, 0, 1, 1, 1, 0, 1),
    (1, 0, 1, 1, 1, 0, 1),
    (1, 0, 0, 0, 0, 0, 1),
    (1, 1, 1, 1, 1, 1, 1));

  HORIZONTAL_SEPARATION_PATTERN: array[0..0, 0..7] of Integer = (
    (0, 0, 0, 0, 0, 0, 0, 0));

  VERTICAL_SEPARATION_PATTERN: array[0..6, 0..0] of Integer = (
    (0), (0), (0), (0), (0), (0), (0));

  POSITION_ADJUSTMENT_PATTERN: array[0..4, 0..4] of Integer = (
    (1, 1, 1, 1, 1),
    (1, 0, 0, 0, 1),
    (1, 0, 1, 0, 1),
    (1, 0, 0, 0, 1),
    (1, 1, 1, 1, 1));

  // From Appendix E. Table 1, JIS0510X:2004 (p 71). The table was double-checked by komatsu.
  POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE: array[0..39, 0..6] of Integer = (
    (-1, -1, -1, -1,  -1,  -1,  -1),  // Version 1
    ( 6, 18, -1, -1,  -1,  -1,  -1),  // Version 2
    ( 6, 22, -1, -1,  -1,  -1,  -1),  // Version 3
    ( 6, 26, -1, -1,  -1,  -1,  -1),  // Version 4
    ( 6, 30, -1, -1,  -1,  -1,  -1),  // Version 5
    ( 6, 34, -1, -1,  -1,  -1,  -1),  // Version 6
    ( 6, 22, 38, -1,  -1,  -1,  -1),  // Version 7
    ( 6, 24, 42, -1,  -1,  -1,  -1),  // Version 8
    ( 6, 26, 46, -1,  -1,  -1,  -1),  // Version 9
    ( 6, 28, 50, -1,  -1,  -1,  -1),  // Version 10
    ( 6, 30, 54, -1,  -1,  -1,  -1),  // Version 11
    ( 6, 32, 58, -1,  -1,  -1,  -1),  // Version 12
    ( 6, 34, 62, -1,  -1,  -1,  -1),  // Version 13
    ( 6, 26, 46, 66,  -1,  -1,  -1),  // Version 14
    ( 6, 26, 48, 70,  -1,  -1,  -1),  // Version 15
    ( 6, 26, 50, 74,  -1,  -1,  -1),  // Version 16
    ( 6, 30, 54, 78,  -1,  -1,  -1),  // Version 17
    ( 6, 30, 56, 82,  -1,  -1,  -1),  // Version 18
    ( 6, 30, 58, 86,  -1,  -1,  -1),  // Version 19
    ( 6, 34, 62, 90,  -1,  -1,  -1),  // Version 20
    ( 6, 28, 50, 72,  94,  -1,  -1),  // Version 21
    ( 6, 26, 50, 74,  98,  -1,  -1),  // Version 22
    ( 6, 30, 54, 78, 102,  -1,  -1),  // Version 23
    ( 6, 28, 54, 80, 106,  -1,  -1),  // Version 24
    ( 6, 32, 58, 84, 110,  -1,  -1),  // Version 25
    ( 6, 30, 58, 86, 114,  -1,  -1),  // Version 26
    ( 6, 34, 62, 90, 118,  -1,  -1),  // Version 27
    ( 6, 26, 50, 74,  98, 122,  -1),  // Version 28
    ( 6, 30, 54, 78, 102, 126,  -1),  // Version 29
    ( 6, 26, 52, 78, 104, 130,  -1),  // Version 30
    ( 6, 30, 56, 82, 108, 134,  -1),  // Version 31
    ( 6, 34, 60, 86, 112, 138,  -1),  // Version 32
    ( 6, 30, 58, 86, 114, 142,  -1),  // Version 33
    ( 6, 34, 62, 90, 118, 146,  -1),  // Version 34
    ( 6, 30, 54, 78, 102, 126, 150),  // Version 35
    ( 6, 24, 50, 76, 102, 128, 154),  // Version 36
    ( 6, 28, 54, 80, 106, 132, 158),  // Version 37
    ( 6, 32, 58, 84, 110, 136, 162),  // Version 38
    ( 6, 26, 54, 82, 110, 138, 166),  // Version 39
    ( 6, 30, 58, 86, 114, 142, 170)   // Version 40
  );

  // Type info cells at the left top corner.
  TYPE_INFO_COORDINATES: array[0..14, 0..1] of Integer = (
    (8, 0),
    (8, 1),
    (8, 2),
    (8, 3),
    (8, 4),
    (8, 5),
    (8, 7),
    (8, 8),
    (7, 8),
    (5, 8),
    (4, 8),
    (3, 8),
    (2, 8),
    (1, 8),
    (0, 8)
  );

  // From Appendix D in JISX0510:2004 (p. 67)
  VERSION_INFO_POLY = $1f25;  // 1 1111 0010 0101

  // From Appendix C in JISX0510:2004 (p.65).
  TYPE_INFO_POLY = $537;
  TYPE_INFO_MASK_PATTERN = $5412;


  VERSION_DECODE_INFO: array[0..33] of Integer = (

      $07C94, $085BC, $09A99, $0A4D3, $0BBF6,
      $0C762, $0D847, $0E60D, $0F928, $10B78,
      $1145D, $12A17, $13532, $149A6, $15683,
      $168C9, $177EC, $18EC4, $191E1, $1AFAB,
      $1B08E, $1CC1A, $1D33F, $1ED75, $1F250,
      $209D5, $216F0, $228BA, $2379F, $24B0B,
      $2542E, $26A64, $27541, $28C69);

type
  TMode = (qmTerminator, qmNumeric, qmAlphanumeric, qmStructuredAppend,
    qmByte, qmECI, qmKanji, qmFNC1FirstPosition, qmFNC1SecondPosition,
    qmHanzi);

const
  ModeCharacterCountBits: array[TMode] of array[0..2] of Integer = (
    (0, 0, 0), (10, 12, 14), (9, 11, 13), (0, 0, 0), (8, 16, 16),
    (0, 0, 0), (8, 10, 12), (0, 0, 0), (0, 0, 0), (8, 10, 12));

  ModeBits: array[TMode] of Integer = (0, 1, 2, 3, 4, 7, 8, 5, 9, 13);

type
  TErrorCorrectionLevel = class
  private
    FBits: Integer;
  public
    procedure Assign(Source: TErrorCorrectionLevel);
    function Ordinal: Integer;
    property Bits: Integer read FBits;
  end;

  TECB = class
  private
    Count: Integer;
    DataCodewords: Integer;
  public
    constructor Create(Count, DataCodewords: Integer);
    function GetCount: Integer;
    function GetDataCodewords: Integer;
  end;

  TECBArray = array of TECB;

  TECBlocks = class
  private
    ECCodewordsPerBlock: Integer;
    ECBlocks: TECBArray;
  public
    constructor Create(ECCodewordsPerBlock: Integer; ECBlocks: TECB); overload;
    constructor Create(ECCodewordsPerBlock: Integer; ECBlocks1, ECBlocks2: TECB); overload;
    destructor Destroy; override;
    function GetTotalECCodewords: Integer;
    function GetNumBlocks: Integer;
    function GetECCodewordsPerBlock: Integer;
    function GetECBlocks: TECBArray;
  end;

  TByteMatrix = class
  protected
    Bytes: T2DByteArray;
    FWidth: Integer;
    FHeight: Integer;
  public
    constructor Create(Width, Height: Integer);
    function Get(X, Y: Integer): Integer;
    procedure SetBoolean(X, Y: Integer; Value: Boolean);
    procedure SetInteger(X, Y: Integer; Value: Integer);
    function GetArray: T2DByteArray;
    procedure Assign(Source: TByteMatrix);
    procedure Clear(Value: Byte);
    function Hash: AnsiString;
    property Width: Integer read FWidth;
    property Height: Integer read FHeight;
  end;

  TBitArray = class
  private
    Bits: array of Integer;
    Size: Integer;
    procedure EnsureCapacity(Size: Integer);
  public
    constructor Create; overload;
    constructor Create(Size: Integer); overload;
    function GetSizeInBytes: Integer;
    function GetSize: Integer;
    function Get(I: Integer): Boolean;
    procedure SetBit(Index: Integer);
    procedure AppendBit(Bit: Boolean);
    procedure AppendBits(Value, NumBits: Integer);
    procedure AppendBitArray(NewBitArray: TBitArray);
    procedure ToBytes(BitOffset: Integer; Source: TByteArray; Offset,
      NumBytes: Integer);
    procedure XorOperation(Other: TBitArray);
  end;

  TCharacterSetECI = class

  end;

  TVersion = class
  private
    VersionNumber: Integer;
    AlignmentPatternCenters: array of Integer;
    ECBlocks: array of TECBlocks;
    TotalCodewords: Integer;
    ECCodewords: Integer;
  public
    constructor Create(VersionNumber: Integer; AlignmentPatternCenters: array of Integer; ECBlocks1, ECBlocks2, ECBlocks3, ECBlocks4: TECBlocks);
    destructor Destroy; override;
    class function GetVersionForNumber(VersionNum: Integer): TVersion;
    class function ChooseVersion(NumInputBits: Integer; ecLevel: TErrorCorrectionLevel): TVersion;
    function GetTotalCodewords: Integer;
    function GetECBlocksForLevel(ECLevel: TErrorCorrectionLevel): TECBlocks;
    function GetDimensionForVersion: Integer;
  end;

  TMaskUtil = class
  public
    function GetDataMaskBit(MaskPattern, X, Y: Integer): Boolean;
  end;

  TQRCode = class
  private
    FMode: TMode;
    FECLevel: TErrorCorrectionLevel;
    FVersion: Integer;
    FMatrixWidth: Integer;
    FMaskPattern: Integer;
    FNumTotalBytes: Integer;
    FNumDataBytes: Integer;
    FNumECBytes: Integer;
    FNumRSBlocks: Integer;
    FMatrix: TByteMatrix;
    FQRCodeError: Boolean;
  public
    constructor Create;
    destructor Destroy; override;
    function At(X, Y: Integer): Integer;
    function IsValid: Boolean;
    function IsValidMaskPattern(MaskPattern: Integer): Boolean;
    procedure SetMatrix(NewMatrix: TByteMatrix);
    procedure SetECLevel(NewECLevel: TErrorCorrectionLevel);
    procedure SetAll(VersionNum, NumBytes, NumDataBytes, NumRSBlocks, NumECBytes, MatrixWidth: Integer);
    property QRCodeError: Boolean read FQRCodeError;
    property Mode: TMode read FMode write FMode;
    property Version: Integer read FVersion write FVersion;
    property NumDataBytes: Integer read FNumDataBytes;
    property NumTotalBytes: Integer read FNumTotalBytes;
    property NumRSBlocks: Integer read FNumRSBlocks;
    property MatrixWidth: Integer read FMatrixWidth;
    property MaskPattern: Integer read FMaskPattern write FMaskPattern;
    property ECLevel: TErrorCorrectionLevel read FECLevel;
  end;

  TMatrixUtil = class

  private
    FMatrixUtilError: Boolean;
    procedure ClearMatrix(Matrix: TByteMatrix);

    procedure EmbedBasicPatterns(Version: Integer; Matrix: TByteMatrix);
    procedure EmbedTypeInfo(ECLevel: TErrorCorrectionLevel; MaskPattern: Integer; Matrix: TByteMatrix);
    procedure MaybeEmbedVersionInfo(Version: Integer; Matrix: TByteMatrix);
    procedure EmbedDataBits(DataBits: TBitArray; MaskPattern: Integer; Matrix: TByteMatrix);
    function FindMSBSet(Value: Integer): Integer;
    function CalculateBCHCode(Value, Poly: Integer): Integer;
    procedure MakeTypeInfoBits(ECLevel: TErrorCorrectionLevel; MaskPattern: Integer; Bits: TBitArray);
    procedure MakeVersionInfoBits(Version: Integer; Bits: TBitArray);
    function IsEmpty(Value: Integer): Boolean;
    procedure EmbedTimingPatterns(Matrix: TByteMatrix);
    procedure EmbedDarkDotAtLeftBottomCorner(Matrix: TByteMatrix);
    procedure EmbedHorizontalSeparationPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
    procedure EmbedVerticalSeparationPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
    procedure EmbedPositionAdjustmentPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
    procedure EmbedPositionDetectionPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
    procedure EmbedPositionDetectionPatternsAndSeparators(Matrix: TByteMatrix);
    procedure MaybeEmbedPositionAdjustmentPatterns(Version: Integer; Matrix: TByteMatrix);
  public
    constructor Create;
    property MatrixUtilError: Boolean read FMatrixUtilError;
    procedure BuildMatrix(DataBits: TBitArray; ECLevel: TErrorCorrectionLevel; Version, MaskPattern: Integer; Matrix: TByteMatrix);
  end;

function GetModeBits(Mode: TMode): Integer;
begin
  Result := ModeBits[Mode];
end;

function GetModeCharacterCountBits(Mode: TMode; Version: TVersion): Integer;
var
  Number: Integer;
  Offset: Integer;
begin
  Number := Version.VersionNumber;

  if (Number <= 9) then
  begin
    Offset := 0;
  end else
  if (number <= 26) then
  begin
    Offset := 1;
  end else
  begin
    Offset := 2;
  end;
  Result := ModeCharacterCountBits[Mode][Offset];
end;

type
  TBlockPair = class
  private
    FDataBytes: TByteArray;
    FErrorCorrectionBytes: TByteArray;
  public
    constructor Create(BA1, BA2: TByteArray);
    function GetDataBytes: TByteArray;
    function GetErrorCorrectionBytes: TByteArray;
  end;

  TGenericGFPoly = class;

  TGenericGF = class
  private
    FExpTable: TIntegerArray;
    FLogTable: TIntegerArray;
    FZero: TGenericGFPoly;
    FOne: TGenericGFPoly;
    FSize: Integer;
    FPrimitive: Integer;
    FGeneratorBase: Integer;
    FInitialized: Boolean;
    FPolyList: array of TGenericGFPoly;

    procedure CheckInit;
    procedure Initialize;
  public
    class function CreateQRCodeField256: TGenericGF;
    class function AddOrSubtract(A, B: Integer): Integer;
    constructor Create(Primitive, Size, B: Integer);
    destructor Destroy; override;
    function GetZero: TGenericGFPoly;
    function Exp(A: Integer): Integer;
    function GetGeneratorBase: Integer;
    function Inverse(A: Integer): Integer;
    function Multiply(A, B: Integer): Integer;
    function BuildMonomial(Degree, Coefficient: Integer): TGenericGFPoly;
  end;

  TGenericGFPolyArray = array of TGenericGFPoly;
  TGenericGFPoly = class
  private
    FField: TGenericGF;
    FCoefficients: TIntegerArray;
  public
    constructor Create(AField: TGenericGF; ACoefficients: TIntegerArray);
    destructor Destroy; override;
    function Coefficients: TIntegerArray;
    function Multiply(Other: TGenericGFPoly): TGenericGFPoly;
    function MultiplyByMonomial(Degree, Coefficient: Integer): TGenericGFPoly;
    function Divide(Other: TGenericGFPoly): TGenericGFPolyArray;
    function GetCoefficients: TIntegerArray;
    function IsZero: Boolean;
    function GetCoefficient(Degree: Integer): Integer;
    function GetDegree: Integer;
    function AddOrSubtract(Other: TGenericGFPoly): TGenericGFPoly;
  end;

  TReedSolomonEncoder = class
  private
    FField: TGenericGF;
    FCachedGenerators: TObjectList;
  public
    constructor Create(AField: TGenericGF);
    destructor Destroy; override;
    procedure Encode(ToEncode: TIntegerArray; ECBytes: Integer);
    function BuildGenerator(Degree: Integer): TGenericGFPoly;
  end;

  TEncoder = class
  private
    FEncoderError: Boolean;

    function ApplyMaskPenaltyRule1Internal(Matrix: TByteMatrix;
      IsHorizontal: Boolean): Integer;
    function ChooseMode(const Content: WideString; var EncodeOptions: Integer): TMode; overload;
    function FilterContent(const Content: WideString; Mode: TMode; EncodeOptions: Integer): WideString;
    procedure Append8BitBytes(const Content: WideString; Bits: TBitArray; EncodeOptions: Integer);

    procedure AppendAlphanumericBytes(const Content: WideString;
      Bits: TBitArray);
    procedure AppendBytes(const Content: WideString; Mode: TMode;
      Bits: TBitArray; EncodeOptions: Integer);
    procedure AppendKanjiBytes(const Content: WideString; Bits: TBitArray);
    procedure AppendLengthInfo(NumLetters, VersionNum: Integer; Mode: TMode;
      Bits: TBitArray);
    procedure AppendModeInfo(Mode: TMode; Bits: TBitArray);
    procedure AppendNumericBytes(const Content: WideString; Bits: TBitArray);
    function ChooseMaskPattern(Bits: TBitArray; ECLevel: TErrorCorrectionLevel;
      Version: Integer; Matrix: TByteMatrix): Integer;
    function GenerateECBytes(DataBytes: TByteArray;

      NumECBytesInBlock: Integer): TByteArray;
    function GetAlphanumericCode(Code: Integer): Integer;
    procedure GetNumDataBytesAndNumECBytesForBlockID(NumTotalBytes,
      NumDataBytes, NumRSBlocks, BlockID: Integer; var NumDataBytesInBlock: TIntegerArray;
      var NumECBytesInBlock: TIntegerArray);
    procedure InterleaveWithECBytes(Bits: TBitArray; NumTotalBytes,
      NumDataBytes, NumRSBlocks: Integer; var Result: TBitArray);
    //function IsOnlyDoubleByteKanji(const Content: WideString): Boolean;
    procedure TerminateBits(NumDataBytes: Integer; var Bits: TBitArray);
    function CalculateMaskPenalty(Matrix: TByteMatrix): Integer;
    function ApplyMaskPenaltyRule1(Matrix: TByteMatrix): Integer;
    function ApplyMaskPenaltyRule2(Matrix: TByteMatrix): Integer;
    function ApplyMaskPenaltyRule3(Matrix: TByteMatrix): Integer;
    function ApplyMaskPenaltyRule4(Matrix: TByteMatrix): Integer;
    //procedure Encode(const Content: WideString; ECLevel: TErrorCorrectionLevel; QRCode: TQRCode); overload;
    procedure Encode(const Content: WideString; EncodeOptions: Integer; ECLevel: TErrorCorrectionLevel; QRCode: TQRCode);
  public
    constructor Create;
    property EncoderError: Boolean read FEncoderError;
  end;

function TEncoder.ApplyMaskPenaltyRule1(Matrix: TByteMatrix): Integer;
begin
  Result := ApplyMaskPenaltyRule1Internal(Matrix, True) +
    ApplyMaskPenaltyRule1Internal(Matrix, False);
end;

// Apply mask penalty rule 2 and return the penalty. Find 2x2 blocks with the same color and give
// penalty to them.
function TEncoder.ApplyMaskPenaltyRule2(Matrix: TByteMatrix): Integer;
var
  Penalty: Integer;
  TheArray: T2DByteArray;
  Width: Integer;
  Height: Integer;
  X: Integer;
  Y: Integer;
  Value: Integer;
begin
  Penalty := 0;
  TheArray := Matrix.GetArray;
  Width := Matrix.Width;
  Height := Matrix.Height;
  for Y := 0 to Height - 2 do
  begin
    for X := 0 to Width - 2 do
    begin
      Value := TheArray[Y][X];
      if ((Value = TheArray[Y][X + 1]) and (Value = TheArray[Y + 1][X]) and
        (Value = TheArray[Y + 1][X + 1])) then
      begin
        Inc(Penalty, 3);
      end;
    end;
  end;
  Result := Penalty;
end;

// Apply mask penalty rule 3 and return the penalty. Find consecutive cells of 00001011101 or
// 10111010000, and give penalty to them.  If we find patterns like 000010111010000, we give
// penalties twice (i.e. 40 * 2).
function TEncoder.ApplyMaskPenaltyRule3(Matrix: TByteMatrix): Integer;
var
  Penalty: Integer;
  TheArray: T2DByteArray;
  Width: Integer;
  Height: Integer;
  X: Integer;
  Y: Integer;
begin
  Penalty := 0;
  TheArray := Matrix.GetArray;
  Width := Matrix.Width;
  Height := Matrix.Height;
  for Y := 0 to Height - 1 do
  begin
    for X := 0 to Width - 1 do
    begin
      if ((X + 6 < Width) and
          (TheArray[Y][X] = 1) and
          (TheArray[Y][X + 1] = 0) and
          (TheArray[Y][X + 2] = 1) and
          (TheArray[Y][X + 3] = 1) and
          (TheArray[Y][X + 4] = 1) and
          (TheArray[Y][X + 5] = 0) and
          (TheArray[Y][X + 6] = 1) and
          (((X + 10 < Width) and
              (TheArray[Y][X + 7] = 0) and
              (TheArray[Y][X + 8] = 0) and
              (TheArray[Y][X + 9] = 0) and
              (TheArray[Y][X + 10] = 0)) or
              ((x - 4 >= 0) and
                  (TheArray[Y][X -  1] = 0) and
                  (TheArray[Y][X -  2] = 0) and
                  (TheArray[Y][X -  3] = 0) and
                  (TheArray[Y][X -  4] = 0)))) then
      begin
        Inc(Penalty, 40);
      end;
      if ((Y + 6 < Height) and
          (TheArray[Y][X] = 1)  and
          (TheArray[Y + 1][X] = 0) and
          (TheArray[Y + 2][X] = 1)  and
          (TheArray[Y + 3][X] = 1)  and
          (TheArray[Y + 4][X] = 1)  and
          (TheArray[Y + 5][X] = 0) and
          (TheArray[Y + 6][X] = 1) and
          (((Y + 10 < Height) and
              (TheArray[Y + 7][X] = 0) and
              (TheArray[Y + 8][X] = 0) and
              (TheArray[Y + 9][X] = 0) and
              (TheArray[Y + 10][X] = 0)) or
              ((Y - 4 >= 0) and
                  (TheArray[Y - 1][X] = 0) and
                  (TheArray[Y - 2][X] = 0) and
                  (TheArray[Y - 3][X] = 0) and
                  (TheArray[Y - 4][X] = 0)))) then
      begin
        Inc(Penalty, 40);
      end;
    end;
  end;
  Result := Penalty;
end;

// Apply mask penalty rule 4 and return the penalty. Calculate the ratio of dark cells and give
// penalty if the ratio is far from 50%. It gives 10 penalty for 5% distance. Examples:
// -   0% => 100
// -  40% =>  20
// -  45% =>  10
// -  50% =>   0
// -  55% =>  10
// -  55% =>  20
// - 100% => 100
function TEncoder.ApplyMaskPenaltyRule4(Matrix: TByteMatrix): Integer;
var
  NumDarkCells: Integer;
  TheArray: T2DByteArray;
  Width: Integer;
  Height: Integer;
  NumTotalCells: Integer;
  DarkRatio: Double;
  X: Integer;
  Y: Integer;
begin
  NumDarkCells := 0;
  TheArray := Matrix.GetArray;
  Width := Matrix.Width;
  Height := matrix.Height;
  for Y := 0 to Height - 1 do
  begin
    for X := 0 to Width - 1 do
    begin
      if (TheArray[Y][X] = 1) then
      begin
        Inc(NumDarkCells);
      end;
    end;
  end;
  numTotalCells := matrix.Height * Matrix.Width;
  DarkRatio := NumDarkCells / NumTotalCells;
  Result := Round(Abs((DarkRatio * 100 - 50)) / 50);
end;

// Helper function for applyMaskPenaltyRule1. We need this for doing this calculation in both
// vertical and horizontal orders respectively.
function TEncoder.ApplyMaskPenaltyRule1Internal(Matrix: TByteMatrix; IsHorizontal: Boolean): Integer;
var
  Penalty: Integer;
  NumSameBitCells: Integer;
  PrevBit: Integer;
  TheArray: T2DByteArray;
  I: Integer;
  J: Integer;
  Bit: Integer;
  ILimit: Integer;
  JLimit: Integer;
begin
  Penalty := 0;
  NumSameBitCells := 0;
  PrevBit := -1;
  // Horizontal mode:
  //   for (int i = 0; i < matrix.height(); ++i) {
  //     for (int j = 0; j < matrix.width(); ++j) {
  //       int bit = matrix.get(i, j);
  // Vertical mode:
  //   for (int i = 0; i < matrix.width(); ++i) {
  //     for (int j = 0; j < matrix.height(); ++j) {
  //       int bit = matrix.get(j, i);
  if (IsHorizontal) then
  begin
    ILimit := Matrix.Height;
    JLimit := Matrix.Width;
  end else
  begin
    ILimit := Matrix.Width;
    JLimit := Matrix.Height;
  end;
  TheArray := Matrix.GetArray;

  for I := 0 to ILimit - 1 do
  begin
    for J := 0 to JLimit - 1 do
    begin
      if (IsHorizontal) then
      begin
        Bit := TheArray[I][J];
      end else
      begin
        Bit := TheArray[J][I];
      end;
      if (Bit = PrevBit) then
      begin
        Inc(NumSameBitCells);
        // Found five repetitive cells with the same color (bit).
        // We'll give penalty of 3.
        if (NumSameBitCells = 5) then
        begin
          Inc(Penalty, 3);
        end else if (NumSameBitCells > 5) then
        begin
          // After five repetitive cells, we'll add the penalty one
          // by one.
          Inc(Penalty, 1);;
        end;
      end else
      begin
        NumSameBitCells := 1;  // Include the cell itself.
        PrevBit := bit;
      end;
    end;
    NumSameBitCells := 0;  // Clear at each row/column.
  end;
  Result := Penalty;
end;

{ TQRCode }

constructor TQRCode.Create;
begin
  FMode := qmTerminator;
  FQRCodeError := False;
  FECLevel := nil;
  FVersion := -1;
  FMatrixWidth := -1;
  FMaskPattern := -1;
  FNumTotalBytes := -1;
  FNumDataBytes := -1;
  FNumECBytes := -1;
  FNumRSBlocks := -1;
  FMatrix := nil;
end;

destructor TQRCode.Destroy;
begin
  if (Assigned(FECLevel)) then
  begin
    FECLevel.Free;
  end;
  if (Assigned(FMatrix)) then
  begin
    FMatrix.Free;
  end;
  inherited;
end;

function TQRCode.At(X, Y: Integer): Integer;
var
  Value: Integer;
begin
  // The value must be zero or one.
  Value := FMatrix.Get(X, Y);
  if (not ((Value = 0) or (Value = 1))) then
  begin
    FQRCodeError := True;
  end;
  Result := Value;
end;

function TQRCode.IsValid: Boolean;
begin
  Result :=
    // First check if all version are not uninitialized.
    ((FECLevel <> nil) and
    (FVersion <> -1) and
    (FMatrixWidth <> -1) and
    (FMaskPattern <> -1) and
    (FNumTotalBytes <> -1) and
    (FNumDataBytes <> -1) and
    (FNumECBytes <> -1) and
    (FNumRSBlocks <> -1) and
    // Then check them in other ways..
    IsValidMaskPattern(FMaskPattern) and
    (FNumTotalBytes = FNumDataBytes + FNumECBytes) and
    // ByteMatrix stuff.
    (Assigned(FMatrix)) and
    (FMatrixWidth = FMatrix.Width) and
    // See 7.3.1 of JISX0510:2004 (Fp.5).
    (FMatrix.Width = FMatrix.Height)); // Must be square.
end;

function TQRCode.IsValidMaskPattern(MaskPattern: Integer): Boolean;
begin
  Result := (MaskPattern >= 0) and (MaskPattern < NUM_MASK_PATTERNS);
end;

procedure TQRCode.SetMatrix(NewMatrix: TByteMatrix);
begin
  if (Assigned(FMatrix)) then
  begin
    FMatrix.Free;
    FMatrix := nil;
  end;
  FMatrix := NewMatrix;
end;

procedure TQRCode.SetAll(VersionNum, NumBytes, NumDataBytes, NumRSBlocks,
  NumECBytes, MatrixWidth: Integer);
begin
  FVersion := VersionNum;
  FNumTotalBytes := NumBytes;
  FNumDataBytes := NumDataBytes;
  FNumRSBlocks := NumRSBlocks;
  FNumECBytes := NumECBytes;
  FMatrixWidth := MatrixWidth;
end;

procedure TQRCode.SetECLevel(NewECLevel: TErrorCorrectionLevel);
begin
  if (Assigned(FECLevel)) then
  begin
    FECLevel.Free;
  end;
  FECLevel := TErrorCorrectionLevel.Create;
  FECLevel.Assign(NewECLevel);
end;

{ TByteMatrix }

procedure TByteMatrix.Clear(Value: Byte);
var
  X, Y: Integer;
begin
  for Y := 0 to FHeight - 1 do
  begin
    for X := 0 to FWidth - 1 do
    begin
      Bytes[Y][X] := Value;
    end;
  end;
end;

constructor TByteMatrix.Create(Width, Height: Integer);
var
  Y: Integer;
  X: Integer;
begin
  FWidth := Width;
  FHeight := Height;
  SetLength(Bytes, Height);
  for Y := 0 to Height - 1 do
  begin
    SetLength(Bytes[Y], Width);
    for X := 0 to Width - 1 do
    begin
      Bytes[Y][X] := 0;
    end;
  end;
end;

function TByteMatrix.Get(X, Y: Integer): Integer;
begin
  if (Bytes[Y][X] = 255) then Result := -1 else Result := Bytes[Y][X];
end;

function TByteMatrix.GetArray: T2DByteArray;
begin
  Result := Bytes;
end;

function TByteMatrix.Hash: AnsiString;
var
  X, Y: Integer;
  Counter: Integer;
  CC: Integer;
begin
  Result := '';
  for Y := 0 to FHeight - 1 do
  begin
    Counter := 0;
    for X := 0 to FWidth - 1 do
    begin
      CC := Get(X, Y);
      if (CC = -1) then CC := 255;
      Counter := Counter + CC;
    end;
    Result := Result + AnsiChar((Counter mod 26) + 65);
  end;
end;

procedure TByteMatrix.SetBoolean(X, Y: Integer; Value: Boolean);
begin
  Bytes[Y][X] := Byte(Value) and $FF;
end;

procedure TByteMatrix.SetInteger(X, Y, Value: Integer);
begin
  Bytes[Y][X] := Value and $FF;
end;

procedure TByteMatrix.Assign(Source: TByteMatrix);
var
  SourceLength: Integer;
begin
  SourceLength := Length(Source.Bytes);
  SetLength(Bytes, SourceLength);
  if (SourceLength > 0) then
  begin
    Move(Source.Bytes[0], Bytes[0], SourceLength);
  end;
  FWidth := Source.Width;
  FHeight := Source.Height;
end;

{ TEncoder }

function TEncoder.CalculateMaskPenalty(Matrix: TByteMatrix): Integer;
var
  Penalty: Integer;
begin
  Penalty := 0;
  Inc(Penalty, ApplyMaskPenaltyRule1(Matrix));
  Inc(Penalty, ApplyMaskPenaltyRule2(Matrix));
  Inc(Penalty, ApplyMaskPenaltyRule3(Matrix));
  Inc(Penalty, ApplyMaskPenaltyRule4(Matrix));
  Result := Penalty;
end;

{procedure TEncoder.Encode(const Content: WideString; ECLevel: TErrorCorrectionLevel; QRCode: TQRCode);
begin
  Encode(Content, ECLevel, nil, QRCode);
end;}

procedure TEncoder.Encode(const Content: WideString; EncodeOptions: Integer; ECLevel: TErrorCorrectionLevel; QRCode: TQRCode);
var
  Mode: TMode;
  DataBits: TBitArray;
  FinalBits: TBitArray;
  HeaderBits: TBitArray;
  HeaderAndDataBits: TBitArray;
  Matrix: TByteMatrix;
  NumLetters: Integer;
  MatrixUtil: TMatrixUtil;
  BitsNeeded: Integer;
  ProvisionalBitsNeeded: Integer;
  ProvisionalVersion: TVersion;
  Version: TVersion;
  ECBlocks: TECBlocks;
  NumDataBytes: Integer;
  Dimension: Integer;
  FilteredContent: WideString;
begin
  DataBits := TBitArray.Create;
  HeaderBits := TBitArray.Create;

  // Pick an encoding mode appropriate for the content. Note that this will not attempt to use
  // multiple modes / segments even if that were more efficient. Twould be nice.
  // Collect data within the main segment, separately, to count its size if needed. Don't add it to
  // main payload yet.

  Mode := ChooseMode(Content, EncodeOptions);
  FilteredContent := FilterContent(Content, Mode, EncodeOptions);
  AppendBytes(FilteredContent, Mode, DataBits, EncodeOptions);

  // (With ECI in place,) Write the mode marker
  AppendModeInfo(Mode, HeaderBits);

  // Hard part: need to know version to know how many bits length takes. But need to know how many
  // bits it takes to know version. First we take a guess at version by assuming version will be
  // the minimum, 1:
  ProvisionalVersion := TVersion.GetVersionForNumber(1);
  try
    ProvisionalBitsNeeded := HeaderBits.GetSize +
      GetModeCharacterCountBits(Mode, ProvisionalVersion) +
      DataBits.GetSize;
  finally
    ProvisionalVersion.Free;
  end;

  ProvisionalVersion := TVersion.ChooseVersion(ProvisionalBitsNeeded, ECLevel);
  try
    // Use that guess to calculate the right version. I am still not sure this works in 100% of cases.
    BitsNeeded := HeaderBits.GetSize +
      GetModeCharacterCountBits(Mode, ProvisionalVersion) +
      DataBits.GetSize;
    Version := TVersion.ChooseVersion(BitsNeeded, ECLevel);
  finally
    ProvisionalVersion.Free;
  end;

  HeaderAndDataBits := TBitArray.Create;
  FinalBits := TBitArray.Create;
  try
    HeaderAndDataBits.AppendBitArray(HeaderBits);

    // Find "length" of main segment and write it
    if (Mode = qmByte) then
    begin
      NumLetters := DataBits.GetSizeInBytes;
    end else
    begin
      NumLetters := Length(FilteredContent);
    end;
    AppendLengthInfo(NumLetters, Version.VersionNumber, Mode, HeaderAndDataBits);
    // Put data together into the overall payload
    HeaderAndDataBits.AppendBitArray(DataBits);

    ECBlocks := Version.GetECBlocksForLevel(ECLevel);
    NumDataBytes := Version.GetTotalCodewords - ECBlocks.GetTotalECCodewords;

    // Terminate the bits properly.
    TerminateBits(NumDataBytes, HeaderAndDataBits);

    // Interleave data bits with error correction code.
    InterleaveWithECBytes(HeaderAndDataBits, Version.GetTotalCodewords,
      NumDataBytes, ECBlocks.GetNumBlocks, FinalBits);

    // QRCode qrCode = new QRCode();  // This is passed in


    QRCode.SetECLevel(ECLevel);
    QRCode.Mode := Mode;
    QRCode.Version := Version.VersionNumber;

    //  Choose the mask pattern and set to "qrCode".
    Dimension := Version.GetDimensionForVersion;
    Matrix := TByteMatrix.Create(Dimension, Dimension);

    QRCode.MaskPattern := ChooseMaskPattern(FinalBits, ECLevel, Version.VersionNumber, Matrix);

    Matrix.Free;
    Matrix := TByteMatrix.Create(Dimension, Dimension);

    // Build the matrix and set it to "qrCode".
    MatrixUtil := TMatrixUtil.Create;
    try
      MatrixUtil.BuildMatrix(FinalBits, QRCode.ECLevel, QRCode.Version,
        QRCode.MaskPattern, Matrix);
    finally
      MatrixUtil.Free;
    end;

    QRCode.SetMatrix(Matrix);  // QRCode will free the matrix
  finally
    DataBits.Free;
    HeaderAndDataBits.Free;
    FinalBits.Free;
    HeaderBits.Free;
    Version.Free;
  end;
end;

function TEncoder.FilterContent(const Content: WideString; Mode: TMode;
  EncodeOptions: Integer): WideString;
var
  X: Integer;
  CanAdd: Boolean;
begin
  Result := '';
  for X := 1 to Length(Content) do
  begin
    CanAdd := False;
    if (Mode = qmNumeric) then
    begin
      CanAdd :=  (Content[X] >= '0') and (Content[X] <= '9');
    end else
    if (Mode = qmAlphanumeric) then
    begin
      CanAdd := GetAlphanumericCode(Ord(Content[X])) > 0;
    end else
    if (Mode = qmByte) then
    begin
      if (EncodeOptions = 3) then
      begin
        CanAdd := Ord(Content[X]) <= $FF;
      end else
      if ((EncodeOptions = 4) or (EncodeOptions = 5)) then
      begin
        CanAdd := True;
      end;
    end;
    if (CanAdd) then
    begin
      Result := Result + Content[X];
    end;
  end;
end;

// Return the code point of the table used in alphanumeric mode or
//  -1 if there is no corresponding code in the table.
function TEncoder.GetAlphanumericCode(Code: Integer): Integer;
begin
  if (Code < Length(ALPHANUMERIC_TABLE)) then
  begin
    Result := ALPHANUMERIC_TABLE[Code];
  end else
  begin
    Result := -1;
  end;
end;

// Choose the mode based on the content
function TEncoder.ChooseMode(const Content: WideString; var EncodeOptions: Integer): TMode;
var
  AllNumeric: Boolean;
  AllAlphanumeric: Boolean;
  AllISO: Boolean;
  I: Integer;
  C: WideChar;
begin
  if (EncodeOptions = 0) then
  begin
    AllNumeric := Length(Content) > 0;
    I := 1;
    while (I <= Length(Content)) and (AllNumeric) do
    begin
      C := Content[I];
      if ((C < '0') or (C > '9')) then
      begin
        AllNumeric := False;
      end else
      begin
        Inc(I);
      end;
    end;

    if (not AllNumeric) then
    begin
      AllAlphanumeric := Length(Content) > 0;
      I := 1;
      while (I <= Length(Content)) and (AllAlphanumeric) do
      begin
        C := Content[I];
        if (GetAlphanumericCode(Ord(C)) < 0) then
        begin
          AllAlphanumeric := False;
        end else
        begin
          Inc(I);
        end;
      end;
    end else
    begin
      AllAlphanumeric := False;
    end;

    if (not AllAlphanumeric) then
    begin
      AllISO := Length(Content) > 0;
      I := 1;
      while (I <= Length(Content)) and (AllISO) do
      begin
        C := Content[I];
        if (Ord(C) > $FF) then
        begin
          AllISO := False;
        end else
        begin
          Inc(I);
        end;
      end;
    end else
    begin
      AllISO := False;
    end;

    if (AllNumeric) then
    begin
      Result := qmNumeric;
    end else
    if (AllAlphanumeric) then
    begin
      Result := qmAlphanumeric;
    end else
    if (AllISO) then
    begin
      Result := qmByte;
      EncodeOptions := 3;
    end else
    begin
      Result := qmByte;
      EncodeOptions := 4;
    end;
  end else
  if (EncodeOptions = 1) then
  begin
    Result := qmNumeric;
  end else
  if (EncodeOptions = 2) then
  begin
    Result := qmAlphanumeric;
  end else
  begin
    Result := qmByte;
  end;
end;

constructor TEncoder.Create;
begin
  FEncoderError := False;
end;

{function TEncoder.IsOnlyDoubleByteKanji(const Content: WideString): Boolean;
var
  I: Integer;
  Char1: Integer;
begin
  Result := True;
  I := 0;
  while ((I < Length(Content)) and Result) do
  begin
    Char1 := Ord(Content[I + 1]);
    if (((Char1 < $81) or (Char1 > $9F)) and ((Char1 < $E0) or (Char1 > $EB))) then
    begin
      Result := False;
    end;
  end;
end;}

function TEncoder.ChooseMaskPattern(Bits: TBitArray; ECLevel: TErrorCorrectionLevel; Version: Integer; Matrix: TByteMatrix): Integer;
var
  MinPenalty: Integer;
  BestMaskPattern: Integer;
  MaskPattern: Integer;
  MatrixUtil: TMatrixUtil;
  Penalty: Integer;
begin
  MinPenalty := MaxInt;
  BestMaskPattern := -1;

  // We try all mask patterns to choose the best one.
  for MaskPattern := 0 to NUM_MASK_PATTERNS - 1 do
  begin
    MatrixUtil := TMatrixUtil.Create;
    try
      MatrixUtil.BuildMatrix(Bits, ECLevel, Version, MaskPattern, Matrix);
    finally
      MatrixUtil.Free;
    end;
    Penalty := CalculateMaskPenalty(Matrix);
    if (Penalty < MinPenalty) then
    begin
      MinPenalty := Penalty;
      BestMaskPattern := MaskPattern;
    end;
  end;

  Result := BestMaskPattern;
end;

// Terminate bits as described in 8.4.8 and 8.4.9 of JISX0510:2004 (p.24).
procedure TEncoder.TerminateBits(NumDataBytes: Integer; var Bits: TBitArray);
var
  Capacity: Integer;
  I: Integer;
  NumBitsInLastByte: Integer;
  NumPaddingBytes: Integer;
begin
  Capacity := NumDataBytes shl 3;
  if (Bits.GetSize > Capacity) then
  begin
    FEncoderError := True;
    Exit;
  end;
  I := 0;
  while ((I < 4) and (Bits.GetSize < capacity)) do
  begin
    Bits.AppendBit(False);
    Inc(I);
  end;

  // Append termination bits. See 8.4.8 of JISX0510:2004 (p.24) for details.
  // If the last byte isn't 8-bit aligned, we'll add padding bits.
  NumBitsInLastByte := Bits.GetSize and $07;
  if (NumBitsInLastByte > 0) then
  begin
    for I := numBitsInLastByte to 7 do
    begin
      Bits.AppendBit(False);
    end;
  end;

  // If we have more space, we'll fill the space with padding patterns defined in 8.4.9 (p.24).
  NumPaddingBytes := NumDataBytes - Bits.GetSizeInBytes;
  for I := 0 to NumPaddingBytes - 1 do
  begin
    if ((I and $01) = 0) then
    begin
      Bits.AppendBits($EC, 8);
    end else
    begin
      Bits.AppendBits($11, 8);
    end;
  end;
  if (Bits.GetSize <> Capacity) then
  begin
    FEncoderError := True;
  end;
end;

// Get number of data bytes and number of error correction bytes for block id "blockID". Store
// the result in "numDataBytesInBlock", and "numECBytesInBlock". See table 12 in 8.5.1 of
// JISX0510:2004 (p.30)
procedure TEncoder.GetNumDataBytesAndNumECBytesForBlockID(NumTotalBytes, NumDataBytes,
  NumRSBlocks, BlockID: Integer; var NumDataBytesInBlock: TIntegerArray;
  var NumECBytesInBlock: TIntegerArray);
var
  NumRSBlocksInGroup1: Integer;
  NumRSBlocksInGroup2: Integer;
  NumTotalBytesInGroup1: Integer;
  NumTotalBytesInGroup2: Integer;
  NumDataBytesInGroup1: Integer;
  NumDataBytesInGroup2: Integer;
  NumECBytesInGroup1: Integer;
  NumECBytesInGroup2: Integer;
begin
  if (BlockID >= NumRSBlocks) then
  begin
    FEncoderError := True;
    Exit;
  end;
  // numRsBlocksInGroup2 = 196 % 5 = 1
  NumRSBlocksInGroup2 := NumTotalBytes mod NumRSBlocks;
  // numRsBlocksInGroup1 = 5 - 1 = 4
  NumRSBlocksInGroup1 := NumRSBlocks - NumRSBlocksInGroup2;
  // numTotalBytesInGroup1 = 196 / 5 = 39
  NumTotalBytesInGroup1 := NumTotalBytes div NumRSBlocks;
  // numTotalBytesInGroup2 = 39 + 1 = 40
  NumTotalBytesInGroup2 := NumTotalBytesInGroup1 + 1;
  // numDataBytesInGroup1 = 66 / 5 = 13
  NumDataBytesInGroup1 := NumDataBytes div NumRSBlocks;
  // numDataBytesInGroup2 = 13 + 1 = 14
  NumDataBytesInGroup2 := NumDataBytesInGroup1 + 1;
  // numEcBytesInGroup1 = 39 - 13 = 26
  NumECBytesInGroup1 := NumTotalBytesInGroup1 - NumDataBytesInGroup1;
  // numEcBytesInGroup2 = 40 - 14 = 26
  NumECBytesInGroup2 := NumTotalBytesInGroup2 - NumDataBytesInGroup2;
  // Sanity checks.
  // 26 = 26
  if (NumECBytesInGroup1 <> NumECBytesInGroup2) then
  begin
    FEncoderError := True;
    Exit;
  end;
  // 5 = 4 + 1.
  if (NumRSBlocks <> (NumRSBlocksInGroup1 + NumRSBlocksInGroup2)) then
  begin
    FEncoderError := True;
    Exit;
  end;
  // 196 = (13 + 26) * 4 + (14 + 26) * 1
  if (NumTotalBytes <>
      ((NumDataBytesInGroup1 + NumECBytesInGroup1) * NumRsBlocksInGroup1) +
      ((NumDataBytesInGroup2 + NumECBytesInGroup2) * NumRsBlocksInGroup2)) then
  begin
    FEncoderError := True;
    Exit;
  end;

  if (BlockID < NumRSBlocksInGroup1) then
  begin
    NumDataBytesInBlock[0] := NumDataBytesInGroup1;
    NumECBytesInBlock[0] := numECBytesInGroup1;
  end else
  begin
    NumDataBytesInBlock[0] := NumDataBytesInGroup2;
    NumECBytesInBlock[0] := numEcBytesInGroup2;
  end;
end;

// Interleave "bits" with corresponding error correction bytes. On success, store the result in
// "result". The interleave rule is complicated. See 8.6 of JISX0510:2004 (p.37) for details.
procedure TEncoder.InterleaveWithECBytes(Bits: TBitArray; NumTotalBytes,
  NumDataBytes, NumRSBlocks: Integer; var Result: TBitArray);
var
  DataBytesOffset: Integer;
  MaxNumDataBytes: Integer;
  MaxNumECBytes: Integer;
  Blocks: TObjectList;
  NumDataBytesInBlock: TIntegerArray;
  NumECBytesInBlock: TIntegerArray;
  Size: Integer;
  DataBytes: TByteArray;
  ECBytes: TByteArray;
  I, J: Integer;
  BlockPair: TBlockPair;
begin
  SetLength(ECBytes, 0);

  // "bits" must have "getNumDataBytes" bytes of data.
  if (Bits.GetSizeInBytes <> NumDataBytes) then
  begin
    FEncoderError := True;
    Exit;
  end;

  // Step 1.  Divide data bytes into blocks and generate error correction bytes for them. We'll
  // store the divided data bytes blocks and error correction bytes blocks into "blocks".
  DataBytesOffset := 0;
  MaxNumDataBytes := 0;
  MaxNumEcBytes := 0;

  // Since, we know the number of reedsolmon blocks, we can initialize the vector with the number.
  Blocks := TObjectList.Create(True);
  try
    Blocks.Capacity := NumRSBlocks;

    for I := 0 to NumRSBlocks - 1 do
    begin
      SetLength(NumDataBytesInBlock, 1);
      SetLength(NumECBytesInBlock, 1);
      GetNumDataBytesAndNumECBytesForBlockID(
          NumTotalBytes, NumDataBytes, NumRSBlocks, I,
          NumDataBytesInBlock, NumEcBytesInBlock);

      Size := NumDataBytesInBlock[0];
      SetLength(DataBytes, Size);
      Bits.ToBytes(8 * DataBytesOffset, DataBytes, 0, Size);
      ECBytes := GenerateECBytes(DataBytes, NumEcBytesInBlock[0]);
      BlockPair := TBlockPair.Create(DataBytes, ECBytes);
      Blocks.Add(BlockPair);

      MaxNumDataBytes := Max(MaxNumDataBytes, Size);
      MaxNumECBytes := Max(MaxNumECBytes, Length(ECBytes));
      Inc(DataBytesOffset, NumDataBytesInBlock[0]);
    end;
    if (NumDataBytes <> DataBytesOffset) then
    begin
      FEncoderError := True;
      Exit;
    end;

    // First, place data blocks.
    for I := 0 to MaxNumDataBytes - 1 do
    begin
      for J := 0 to Blocks.Count - 1 do
      begin
        DataBytes := TBlockPair(Blocks.Items[J]).GetDataBytes;
        if (I < Length(DataBytes)) then
        begin
          Result.AppendBits(DataBytes[I], 8);
        end;
      end;
    end;
    // Then, place error correction blocks.
    for I := 0 to MaxNumECBytes - 1 do
    begin
      for J := 0 to Blocks.Count - 1 do
      begin
        ECBytes := TBlockPair(Blocks.Items[J]).GetErrorCorrectionBytes;
        if (I < Length(ECBytes)) then
        begin
          Result.AppendBits(ECBytes[I], 8);
        end;
      end;
    end;
  finally
    Blocks.Free;
  end;
  if (numTotalBytes <> Result.GetSizeInBytes) then  // Should be same.
  begin
    FEncoderError := True;
    Exit;
  end;
end;

function TEncoder.GenerateECBytes(DataBytes: TByteArray; NumECBytesInBlock: Integer): TByteArray;
var
  NumDataBytes: Integer;
  ToEncode: TIntegerArray;
  ReedSolomonEncoder: TReedSolomonEncoder;
  I: Integer;
  ECBytes: TByteArray;
  GenericGF: TGenericGF;
begin
  NumDataBytes := Length(DataBytes);
  SetLength(ToEncode, NumDataBytes + NumECBytesInBlock);

  for I := 0 to NumDataBytes - 1 do
  begin
    ToEncode[I] := DataBytes[I] and $FF;
  end;

  GenericGF := TGenericGF.CreateQRCodeField256;
  try
    ReedSolomonEncoder := TReedSolomonEncoder.Create(GenericGF);
    try
      ReedSolomonEncoder.Encode(ToEncode, NumECBytesInBlock);
    finally
      ReedSolomonEncoder.Free;
    end;
  finally
    GenericGF.Free;
  end;

  SetLength(ECBytes, NumECBytesInBlock);
  for I := 0 to NumECBytesInBlock - 1 do
  begin
    ECBytes[I] := ToEncode[NumDataBytes + I];
  end;

  Result := ECBytes;
end;

// Append mode info. On success, store the result in "bits".
procedure TEncoder.AppendModeInfo(Mode: TMode; Bits: TBitArray);
begin
  Bits.AppendBits(GetModeBits(Mode), 4);
end;

// Append length info. On success, store the result in "bits".
procedure TEncoder.AppendLengthInfo(NumLetters, VersionNum: Integer; Mode: TMode; Bits: TBitArray);
var
  NumBits: Integer;
  Version: TVersion;
begin
  Version := TVersion.GetVersionForNumber(VersionNum);
  try
    NumBits := GetModeCharacterCountBits(Mode, Version);
  finally
    Version.Free;
  end;

  if (NumLetters > ((1 shl NumBits) - 1)) then
  begin
    FEncoderError := True;
    Exit;
  end;

  Bits.AppendBits(NumLetters, NumBits);
end;

// Append "bytes" in "mode" mode (encoding) into "bits". On success, store the result in "bits".
procedure TEncoder.AppendBytes(const Content: WideString; Mode: TMode; Bits: TBitArray; EncodeOptions: Integer);
begin
  if (Mode = qmNumeric) then
  begin
    AppendNumericBytes(Content, Bits);
  end else
  if (Mode = qmAlphanumeric) then
  begin
    AppendAlphanumericBytes(Content, Bits);
  end else
  if (Mode = qmByte) then
  begin
    Append8BitBytes(Content, Bits, EncodeOptions);
  end else
  if (Mode = qmKanji) then
  begin
    AppendKanjiBytes(Content, Bits);
  end else
  begin
    FEncoderError := True;
    Exit;
  end;
end;

procedure TEncoder.AppendNumericBytes(const Content: WideString; Bits: TBitArray);
var
  ContentLength: Integer;
  I: Integer;
  Num1: Integer;
  Num2: Integer;
  Num3: Integer;
begin
  ContentLength := Length(Content);
  I := 0;
  while (I < ContentLength) do
  begin
    Num1 := Ord(Content[I + 0 + 1]) - Ord('0');
    if (I + 2 < ContentLength) then
    begin
      // Encode three numeric letters in ten bits.
      Num2 := Ord(Content[I + 1 + 1]) - Ord('0');
      Num3 := Ord(Content[I + 2 + 1]) - Ord('0');
      Bits.AppendBits(Num1 * 100 + Num2 * 10 + Num3, 10);
      Inc(I, 3);
    end else
    if (I + 1 < ContentLength) then
    begin
      // Encode two numeric letters in seven bits.
      Num2 := Ord(Content[I + 1 + 1]) - Ord('0');
      Bits.AppendBits(Num1 * 10 + Num2, 7);
      Inc(I, 2);
    end else
    begin
      // Encode one numeric letter in four bits.
      Bits.AppendBits(Num1, 4);
      Inc(I);
    end;
  end;
end;

procedure TEncoder.AppendAlphanumericBytes(const Content: WideString; Bits: TBitArray);
var
  ContentLength: Integer;
  I: Integer;
  Code1: Integer;
  Code2: Integer;
begin
  ContentLength := Length(Content);
  I := 0;
  while (I < ContentLength) do
  begin
    Code1 := GetAlphanumericCode(Ord(Content[I + 0 + 1]));
    if (Code1 = -1) then
    begin
      FEncoderError := True;
      Exit;
    end;
    if (I + 1 < ContentLength) then
    begin
      Code2 := GetAlphanumericCode(Ord(Content[I + 1 + 1]));
      if (Code2 = -1) then
      begin
        FEncoderError := True;
        Exit;
      end;
      // Encode two alphanumeric letters in 11 bits.
      Bits.AppendBits(Code1 * 45 + Code2, 11);
      Inc(I, 2);
    end else
    begin
      // Encode one alphanumeric letter in six bits.
      Bits.AppendBits(Code1, 6);
      Inc(I);
    end;
  end;
end;

procedure TEncoder.Append8BitBytes(const Content: WideString; Bits: TBitArray; EncodeOptions: Integer);
var
  Bytes: TByteArray;
  I: Integer;
  UTF8Version: AnsiString;
begin
  SetLength(Bytes, 0);
  if (EncodeOptions = 3) then
  begin
    SetLength(Bytes, Length(Content));
    for I := 1 to Length(Content) do
    begin
      Bytes[I - 1] := Ord(Content[I]) and $FF;
    end;
  end else
  if (EncodeOptions = 4) then
  begin
    // Add the UTF-8 BOM
    UTF8Version := #$EF#$BB#$BF + UTF8Encode(Content);
    SetLength(Bytes, Length(UTF8Version));
    if (Length(UTF8Version) > 0) then
    begin
      Move(UTF8Version[1], Bytes[0], Length(UTF8Version));
    end;
  end else
  if (EncodeOptions = 5) then
  begin
    // No BOM
    UTF8Version := UTF8Encode(Content);
    SetLength(Bytes, Length(UTF8Version));
    if (Length(UTF8Version) > 0) then
    begin
      Move(UTF8Version[1], Bytes[0], Length(UTF8Version));
    end;
  end;
  for I := 0 to Length(Bytes) - 1 do
  begin
    Bits.AppendBits(Bytes[I], 8);
  end;
end;

procedure TEncoder.AppendKanjiBytes(const Content: WideString; Bits: TBitArray);
var
  Bytes: TByteArray;
  ByteLength: Integer;
  I: Integer;
  Byte1: Integer;
  Byte2: Integer;
  Code: Integer;
  Subtracted: Integer;
  Encoded: Integer;
begin
  SetLength(Bytes, 0);
  try

  except
    FEncoderError := True;
    Exit;
  end;

  ByteLength := Length(Bytes);
  I := 0;
  while (I < ByteLength) do
  begin
    Byte1 := Bytes[I] and $FF;
    Byte2 := Bytes[I + 1] and $FF;
    Code := (Byte1 shl 8) or Byte2;
    Subtracted := -1;
    if ((Code >= $8140) and (Code <= $9ffc)) then
    begin
      Subtracted := Code - $8140;
    end else
    if ((Code >= $e040) and (Code <= $ebbf)) then
    begin
      Subtracted := Code - $c140;
    end;
    if (Subtracted = -1) then
    begin
      FEncoderError := True;
      Exit;
    end;
    Encoded := ((Subtracted shr 8) * $c0) + (Subtracted and $ff);
    Bits.AppendBits(Encoded, 13);
    Inc(I, 2);
  end;
end;

procedure TMatrixUtil.ClearMatrix(Matrix: TByteMatrix);
begin
  Matrix.Clear(Byte(-1));
end;

constructor TMatrixUtil.Create;
begin
  FMatrixUtilError := False;
end;

// Build 2D matrix of QR Code from "dataBits" with "ecLevel", "version" and "getMaskPattern". On
// success, store the result in "matrix" and return true.
procedure TMatrixUtil.BuildMatrix(DataBits: TBitArray; ECLevel: TErrorCorrectionLevel;
  Version, MaskPattern: Integer; Matrix: TByteMatrix);
begin
  ClearMatrix(Matrix);
  EmbedBasicPatterns(Version, Matrix);

  // Type information appear with any version.
  EmbedTypeInfo(ECLevel, MaskPattern, Matrix);

  // Version info appear if version >= 7.
  MaybeEmbedVersionInfo(Version, Matrix);

  // Data should be embedded at end.
  EmbedDataBits(DataBits, MaskPattern, Matrix);
end;

// Embed basic patterns. On success, modify the matrix and return true.
// The basic patterns are:
// - Position detection patterns
// - Timing patterns
// - Dark dot at the left bottom corner
// - Position adjustment patterns, if need be
procedure TMatrixUtil.EmbedBasicPatterns(Version: Integer; Matrix: TByteMatrix);
begin
  // Let's get started with embedding big squares at corners.
  EmbedPositionDetectionPatternsAndSeparators(Matrix);

  // Then, embed the dark dot at the left bottom corner.
  EmbedDarkDotAtLeftBottomCorner(Matrix);

  // Position adjustment patterns appear if version >= 2.
  MaybeEmbedPositionAdjustmentPatterns(Version, Matrix);

  // Timing patterns should be embedded after position adj. patterns.
  EmbedTimingPatterns(Matrix);
end;

// Embed type information. On success, modify the matrix.
procedure TMatrixUtil.EmbedTypeInfo(ECLevel: TErrorCorrectionLevel; MaskPattern: Integer; Matrix: TByteMatrix);
var
  TypeInfoBits: TBitArray;
  I: Integer;
  Bit: Boolean;
  X1, Y1: Integer;
  X2, Y2: Integer;
begin
  TypeInfoBits := TBitArray.Create;
  try
    MakeTypeInfoBits(ECLevel, MaskPattern, TypeInfoBits);

    for I := 0 to TypeInfoBits.GetSize - 1 do
    begin
      // Place bits in LSB to MSB order.  LSB (least significant bit) is the last value in
      // "typeInfoBits".
      Bit := TypeInfoBits.Get(TypeInfoBits.GetSize - 1 - I);

      // Type info bits at the left top corner. See 8.9 of JISX0510:2004 (p.46).
      X1 := TYPE_INFO_COORDINATES[I][0];
      Y1 := TYPE_INFO_COORDINATES[I][1];
      Matrix.SetBoolean(X1, Y1, Bit);

      if (I < 8) then
      begin
        // Right top corner.
        X2 := Matrix.Width - I - 1;
        Y2 := 8;
        Matrix.SetBoolean(X2, Y2, Bit);
      end else
      begin
        // Left bottom corner.
        X2 := 8;
        Y2 := Matrix.Height - 7 + (I - 8);
        Matrix.SetBoolean(X2, Y2, Bit);
      end;
    end;
  finally
    TypeInfoBits.Free;
  end;
end;

// Embed version information if need be. On success, modify the matrix and return true.
// See 8.10 of JISX0510:2004 (p.47) for how to embed version information.
procedure TMatrixUtil.MaybeEmbedVersionInfo(Version: Integer; Matrix: TByteMatrix);
var
  VersionInfoBits: TBitArray;
  I, J: Integer;
  BitIndex: Integer;
  Bit: Boolean;
begin
  if (Version < 7) then
  begin
    Exit;  // Don't need version info.
  end;

  VersionInfoBits := TBitArray.Create;
  try
    MakeVersionInfoBits(Version, VersionInfoBits);

    BitIndex := 6 * 3 - 1;  // It will decrease from 17 to 0.
    for I := 0 to 5 do
    begin
      for J := 0 to 2 do
      begin
        // Place bits in LSB (least significant bit) to MSB order.
        Bit := VersionInfoBits.Get(BitIndex);
        Dec(BitIndex);
        // Left bottom corner.
        Matrix.SetBoolean(I, Matrix.Height - 11 + J, Bit);
        // Right bottom corner.
        Matrix.SetBoolean(Matrix.Height - 11 + J, I, bit);
      end;
    end;
  finally
    VersionInfoBits.Free;
  end;
end;

// Embed "dataBits" using "getMaskPattern". On success, modify the matrix and return true.
// For debugging purposes, it skips masking process if "getMaskPattern" is -1.
// See 8.7 of JISX0510:2004 (p.38) for how to embed data bits.
procedure TMatrixUtil.EmbedDataBits(DataBits: TBitArray; MaskPattern: Integer; Matrix: TByteMatrix);
var
  BitIndex: Integer;
  Direction: Integer;
  X, Y, I, XX: Integer;
  Bit: Boolean;
  MaskUtil: TMaskUtil;
begin
  MaskUtil := TMaskUtil.Create;
  try
    bitIndex := 0;
    direction := -1;
    // Start from the right bottom cell.
    X := Matrix.Width - 1;
    Y := Matrix.Height - 1;
    while (X > 0) do
    begin
      // Skip the vertical timing pattern.
      if (X = 6) then
      begin
        Dec(X, 1);
      end;
      while ((Y >= 0) and (y < Matrix.Height)) do
      begin
        for I := 0 to 1 do
        begin
          XX := X - I;
          // Skip the cell if it's not empty.
          if (not IsEmpty(Matrix.Get(XX, Y))) then
          begin
            Continue;
          end;

          if (BitIndex < DataBits.GetSize) then
          begin
            Bit := DataBits.Get(BitIndex);
            Inc(BitIndex);
          end else
          begin
            // Padding bit. If there is no bit left, we'll fill the left cells with 0, as described
            // in 8.4.9 of JISX0510:2004 (p. 24).
            Bit := False;
          end;

          // Skip masking if mask_pattern is -1.
          if (MaskPattern <> -1) then
          begin
            if (MaskUtil.GetDataMaskBit(MaskPattern, XX, Y)) then
            begin
              Bit := not Bit;
            end;
          end;
          Matrix.SetBoolean(XX, Y, Bit);
        end;
        Inc(Y, Direction);
      end;
      Direction := -Direction;  // Reverse the direction.
      Inc(Y, Direction);
      Dec(X, 2);  // Move to the left.
    end;
  finally
    MaskUtil.Free;
  end;

  // All bits should be consumed.
  if (BitIndex <> DataBits.GetSize()) then
  begin
    FMatrixUtilError := True;
    Exit;
  end;
end;

// Return the position of the most significant bit set (to one) in the "value". The most
// significant bit is position 32. If there is no bit set, return 0. Examples:
// - findMSBSet(0) => 0
// - findMSBSet(1) => 1
// - findMSBSet(255) => 8
function TMatrixUtil.FindMSBSet(Value: Integer): Integer;
var
  NumDigits: Integer;
begin
  NumDigits := 0;
  while (Value <> 0) do
  begin
    Value := Value shr 1;
    Inc(NumDigits);
  end;
  Result := NumDigits;
end;

// Calculate BCH (Bose-Chaudhuri-Hocquenghem) code for "value" using polynomial "poly". The BCH
// code is used for encoding type information and version information.
// Example: Calculation of version information of 7.
// f(x) is created from 7.
//   - 7 = 000111 in 6 bits
//   - f(x) = x^2 + x^1 + x^0
// g(x) is given by the standard (p. 67)
//   - g(x) = x^12 + x^11 + x^10 + x^9 + x^8 + x^5 + x^2 + 1
// Multiply f(x) by x^(18 - 6)
//   - f'(x) = f(x) * x^(18 - 6)
//   - f'(x) = x^14 + x^13 + x^12
// Calculate the remainder of f'(x) / g(x)
//         x^2
//         __________________________________________________
//   g(x) )x^14 + x^13 + x^12
//         x^14 + x^13 + x^12 + x^11 + x^10 + x^7 + x^4 + x^2
//         --------------------------------------------------
//                              x^11 + x^10 + x^7 + x^4 + x^2
//
// The remainder is x^11 + x^10 + x^7 + x^4 + x^2
// Encode it in binary: 110010010100
// The return value is 0xc94 (1100 1001 0100)
//
// Since all coefficients in the polynomials are 1 or 0, we can do the calculation by bit
// operations. We don't care if cofficients are positive or negative.
function TMatrixUtil.CalculateBCHCode(Value, Poly: Integer): Integer;
var
  MSBSetInPoly: Integer;
begin
  // If poly is "1 1111 0010 0101" (version info poly), msbSetInPoly is 13. We'll subtract 1
  // from 13 to make it 12.
  MSBSetInPoly := FindMSBSet(Poly);
  Value := Value shl (MSBSetInPoly - 1);
  // Do the division business using exclusive-or operations.
  while (FindMSBSet(Value) >= MSBSetInPoly) do
  begin
    Value := Value xor (Poly shl (FindMSBSet(Value) - MSBSetInPoly));
  end;
  // Now the "value" is the remainder (i.e. the BCH code)
  Result := Value;
end;

// Make bit vector of type information. On success, store the result in "bits" and return true.
// Encode error correction level and mask pattern. See 8.9 of
// JISX0510:2004 (p.45) for details.
procedure TMatrixUtil.MakeTypeInfoBits(ECLevel: TErrorCorrectionLevel; MaskPattern: Integer; Bits: TBitArray);
var
  TypeInfo: Integer;
  BCHCode: Integer;
  MaskBits: TBitArray;
begin
  if ((MaskPattern >= 0) and (MaskPattern < NUM_MASK_PATTERNS)) then
  begin
    TypeInfo := (ECLevel.Bits shl 3) or MaskPattern;
    Bits.AppendBits(TypeInfo, 5);

    BCHCode := CalculateBCHCode(TypeInfo, TYPE_INFO_POLY);
    Bits.AppendBits(BCHCode, 10);

    MaskBits := TBitArray.Create;
    try
      MaskBits.AppendBits(TYPE_INFO_MASK_PATTERN, 15);
      Bits.XorOperation(MaskBits);
    finally
      MaskBits.Free;
    end;

    if (Bits.GetSize <> 15) then  // Just in case.
    begin
      FMatrixUtilError := True;
      Exit;
    end;
  end;
end;

// Make bit vector of version information. On success, store the result in "bits" and return true.
// See 8.10 of JISX0510:2004 (p.45) for details.
procedure TMatrixUtil.MakeVersionInfoBits(Version: Integer; Bits: TBitArray);
var
  BCHCode: Integer;
begin
  Bits.AppendBits(Version, 6);
  BCHCode := CalculateBCHCode(Version, VERSION_INFO_POLY);
  Bits.AppendBits(BCHCode, 12);

  if (Bits.GetSize() <> 18) then
  begin
    FMatrixUtilError := True;
    Exit;
  end;
end;

// Check if "value" is empty.
function TMatrixUtil.IsEmpty(Value: Integer): Boolean;
begin
  Result := (Value = -1);
end;

procedure TMatrixUtil.EmbedTimingPatterns(Matrix: TByteMatrix);
var
  I: Integer;
  Bit: Integer;
begin
  // -8 is for skipping position detection patterns (size 7), and two horizontal/vertical
  // separation patterns (size 1). Thus, 8 = 7 + 1.
  for I := 8 to Matrix.Width - 9 do
  begin
    Bit := (I + 1) mod 2;
    // Horizontal line.
    if (IsEmpty(Matrix.Get(I, 6))) then
    begin
      Matrix.SetInteger(I, 6, Bit);
    end;
    // Vertical line.
    if (IsEmpty(Matrix.Get(6, I))) then
    begin
      Matrix.SetInteger(6, I, Bit);
    end;
  end;
end;

// Embed the lonely dark dot at left bottom corner. JISX0510:2004 (p.46)
procedure TMatrixUtil.EmbedDarkDotAtLeftBottomCorner(Matrix: TByteMatrix);
begin
  if (Matrix.Get(8, Matrix.Height - 8) = 0) then
  begin
    FMatrixUtilError := True;
    Exit;
  end;
  Matrix.SetInteger(8, Matrix.Height - 8, 1);
end;

procedure TMatrixUtil.EmbedHorizontalSeparationPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
var
  X: Integer;
begin
  // We know the width and height.
  for X := 0 to 7 do
  begin
    if (not IsEmpty(Matrix.Get(XStart + X, YStart))) then
    begin
      FMatrixUtilError := True;
      Exit;
    end;
    Matrix.SetInteger(XStart + X, YStart, HORIZONTAL_SEPARATION_PATTERN[0][X]);
  end;
end;

procedure TMatrixUtil.EmbedVerticalSeparationPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
var
  Y: Integer;
begin
  // We know the width and height.
  for Y := 0 to 6 do
  begin
    if (not IsEmpty(Matrix.Get(XStart, YStart + Y))) then
    begin
      FMatrixUtilError := True;
      Exit;
    end;
    Matrix.SetInteger(XStart, YStart + Y, VERTICAL_SEPARATION_PATTERN[Y][0]);
  end;
end;

// Note that we cannot unify the function with embedPositionDetectionPattern() despite they are
// almost identical, since we cannot write a function that takes 2D arrays in different sizes in
// C/C++. We should live with the fact.
procedure TMatrixUtil.EmbedPositionAdjustmentPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
var
  X, Y: Integer;
begin
  // We know the width and height.
  for Y := 0 to 4 do
  begin
    for X := 0 to 4 do
    begin
      if (not IsEmpty(Matrix.Get(XStart + X, YStart + Y))) then
      begin
        FMatrixUtilError := True;
        Exit;
      end;
      Matrix.SetInteger(XStart + X, YStart + Y, POSITION_ADJUSTMENT_PATTERN[Y][X]);
    end;
  end;
end;

procedure TMatrixUtil.EmbedPositionDetectionPattern(XStart, YStart: Integer; Matrix: TByteMatrix);
var
  X, Y: Integer;
begin
  // We know the width and height.
  for Y := 0 to 6 do
  begin
    for X := 0 to 6 do
    begin
      if (not IsEmpty(Matrix.Get(XStart + X, YStart + Y))) then
      begin
        FMatrixUtilError := True;
        Exit;
      end;
      Matrix.SetInteger(XStart + X, YStart + Y, POSITION_DETECTION_PATTERN[Y][X]);
    end;
  end;
end;

// Embed position detection patterns and surrounding vertical/horizontal separators.
procedure TMatrixUtil.EmbedPositionDetectionPatternsAndSeparators(Matrix: TByteMatrix);
var
  PDPWidth: Integer;
  HSPWidth: Integer;
  VSPSize: Integer;
begin
  // Embed three big squares at corners.
  PDPWidth := Length(POSITION_DETECTION_PATTERN[0]);
  // Left top corner.
  EmbedPositionDetectionPattern(0, 0, Matrix);
  // Right top corner.
  EmbedPositionDetectionPattern(Matrix.Width - PDPWidth, 0, Matrix);
  // Left bottom corner.
  EmbedPositionDetectionPattern(0, Matrix.Width- PDPWidth, Matrix);

  // Embed horizontal separation patterns around the squares.
  HSPWidth := Length(HORIZONTAL_SEPARATION_PATTERN[0]);
  // Left top corner.
  EmbedHorizontalSeparationPattern(0, HSPWidth - 1, Matrix);
  // Right top corner.
  EmbedHorizontalSeparationPattern(Matrix.Width - HSPWidth,
      HSPWidth - 1, Matrix);
  // Left bottom corner.
  EmbedHorizontalSeparationPattern(0, Matrix.Width - HSPWidth, Matrix);

  // Embed vertical separation patterns around the squares.
  VSPSize := Length(VERTICAL_SEPARATION_PATTERN);
  // Left top corner.
  EmbedVerticalSeparationPattern(VSPSize, 0, Matrix);
  // Right top corner.
  EmbedVerticalSeparationPattern(Matrix.Height - VSPSize - 1, 0, Matrix);
  // Left bottom corner.
  EmbedVerticalSeparationPattern(VSPSize, Matrix.Height - VSPSize, Matrix);
end;

// Embed position adjustment patterns if need be.
procedure TMatrixUtil.MaybeEmbedPositionAdjustmentPatterns(Version: Integer; Matrix: TByteMatrix);
var
  Index: Integer;
  Coordinates: array of Integer;
  NumCoordinates: Integer;
  X, Y, I, J: Integer;
begin
  if (Version >= 2) then
  begin
    Index := Version - 1;
    NumCoordinates := Length(POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[Index]);
    SetLength(Coordinates, NumCoordinates);
    Move(POSITION_ADJUSTMENT_PATTERN_COORDINATE_TABLE[Index][0], Coordinates[0], NumCoordinates * SizeOf(Integer));
    for I := 0 to NumCoordinates - 1 do
    begin
      for J := 0 to NumCoordinates - 1 do
      begin
        Y := Coordinates[I];
        X := Coordinates[J];
        if ((X = -1) or (Y = -1)) then
        begin
          Continue;
        end;
        // If the cell is unset, we embed the position adjustment pattern here.
        if (IsEmpty(Matrix.Get(X, Y))) then
        begin
          // -2 is necessary since the x/y coordinates point to the center of the pattern, not the
          // left top corner.
          EmbedPositionAdjustmentPattern(X - 2, Y - 2, Matrix);
        end;
      end;
    end;
  end;
end;


{ TBitArray }


procedure TBitArray.AppendBits(Value, NumBits: Integer);
var
  NumBitsLeft: Integer;
begin
  if ((NumBits < 0) or (NumBits > 32)) then
  begin

  end;
  EnsureCapacity(Size + NumBits);
  for NumBitsLeft := NumBits downto 1 do
  begin
    AppendBit(((Value shr (NumBitsLeft - 1)) and $01) = 1);
  end;
end;

constructor TBitArray.Create(Size: Integer);

begin
  Size := Size;
  SetLength(Bits, (Size + 31) shr 5);
end;

constructor TBitArray.Create;
begin
  Size := 0;
  SetLength(Bits, 1);
end;

function TBitArray.Get(I: Integer): Boolean;
begin
  Result := (Bits[I shr 5] and (1 shl (I and $1F))) <> 0;
end;

function TBitArray.GetSize: Integer;
begin
  Result := Size;
end;

function TBitArray.GetSizeInBytes: Integer;
begin
  Result := (Size + 7) shr 3;
end;

procedure TBitArray.SetBit(Index: Integer);
begin
  Bits[Index shr 5] := Bits[Index shr 5] or (1 shl (Index and $1F));
end;

procedure TBitArray.AppendBit(Bit: Boolean);
begin
  EnsureCapacity(Size + 1);
  if (Bit) then
  begin
    Bits[Size shr 5] := Bits[Size shr 5] or (1 shl (Size and $1F));
  end;
  Inc(Size);
end;

procedure TBitArray.ToBytes(BitOffset: Integer; Source: TByteArray; Offset,
  NumBytes: Integer);
var
  I: Integer;
  J: Integer;
  TheByte: Integer;
begin
  for I := 0 to NumBytes - 1 do
  begin
    TheByte := 0;
    for J := 0 to 7 do
    begin
      if (Get(BitOffset)) then
      begin
        TheByte := TheByte or (1 shl (7 - J));
      end;
      Inc(BitOffset);
    end;
    Source[Offset + I] := TheByte;
  end;
end;

procedure TBitArray.XorOperation(Other: TBitArray);
var
  I: Integer;
begin
  if (Length(Bits) = Length(Other.Bits)) then
  begin
    for I := 0 to Length(Bits) - 1 do
    begin
      // The last byte could be incomplete (i.e. not have 8 bits in
      // it) but there is no problem since 0 XOR 0 == 0.
      Bits[I] := Bits[I] xor Other.Bits[I];
    end;
  end;
end;

procedure TBitArray.AppendBitArray(NewBitArray: TBitArray);
var
  OtherSize: Integer;
  I: Integer;
begin
  OtherSize := NewBitArray.GetSize;
  EnsureCapacity(Size + OtherSize);
  for I := 0 to OtherSize - 1 do
  begin
    AppendBit(NewBitArray.Get(I));
  end;
end;

procedure TBitArray.EnsureCapacity(Size: Integer);
begin
  if (Size > (Length(Bits) shl 5)) then
  begin
    SetLength(Bits, Size);
  end;
end;

{ TErrorCorrectionLevel }

procedure TErrorCorrectionLevel.Assign(Source: TErrorCorrectionLevel);
begin
  Self.FBits := Source.FBits;
end;

function TErrorCorrectionLevel.Ordinal: Integer;
begin
  Result := 0;
end;

{ TVersion }

class function TVersion.ChooseVersion(NumInputBits: Integer;
  ECLevel: TErrorCorrectionLevel): TVersion;
var
  VersionNum: Integer;
  Version: TVersion;
  NumBytes: Integer;
  ECBlocks: TECBlocks;
  NumECBytes: Integer;
  NumDataBytes: Integer;
  TotalInputBytes: Integer;
begin
  Result := nil;
  // In the following comments, we use numbers of Version 7-H.
  for VersionNum := 1 to 40 do
  begin
    Version := TVersion.GetVersionForNumber(VersionNum);

    // numBytes = 196
    NumBytes := Version.GetTotalCodewords;
    // getNumECBytes = 130
    ECBlocks := Version.GetECBlocksForLevel(ECLevel);
    NumECBytes := ECBlocks.GetTotalECCodewords;
    // getNumDataBytes = 196 - 130 = 66
    NumDataBytes := NumBytes - NumECBytes;
    TotalInputBytes := (NumInputBits + 7) div 8;

    if (numDataBytes >= totalInputBytes) then
    begin
      Result := Version;
      Exit;
    end else
    begin
      Version.Free;
    end;
  end;
end;

constructor TVersion.Create(VersionNumber: Integer;
  AlignmentPatternCenters: array of Integer; ECBlocks1, ECBlocks2, ECBlocks3,
  ECBlocks4: TECBlocks);
var
  Total: Integer;
  ECBlock: TECB;
  ECBArray: TECBArray;
  I: Integer;
begin
  Self.VersionNumber := VersionNumber;
  SetLength(Self.AlignmentPatternCenters, Length(AlignmentPatternCenters));
  if (Length(AlignmentPatternCenters) > 0) then
  begin
    Move(AlignmentPatternCenters[0], Self.AlignmentPatternCenters[0],
      Length(AlignmentPatternCenters) * SizeOf(Integer));
  end;
  SetLength(ECBlocks, 4);
  ECBlocks[0] := ECBlocks1;
  ECBlocks[1] := ECBlocks2;
  ECBlocks[2] := ECBlocks3;
  ECBlocks[3] := ECBlocks4;
  Total := 0;
  ECCodewords := ECBlocks1.GetECCodewordsPerBlock;
  ECBArray := ECBlocks1.GetECBlocks;
  for I := 0 to Length(ECBArray) - 1 do
  begin
    ECBlock := ECBArray[I];
    Inc(Total, ECBlock.GetCount * (ECBlock.GetDataCodewords + ECCodewords));
  end;
  TotalCodewords := Total;
end;

destructor TVersion.Destroy;
var
  X: Integer;
begin
  for X := 0 to Length(ECBlocks) - 1 do
  begin
    ECBlocks[X].Free;
  end;
  inherited;
end;

function TVersion.GetDimensionForVersion: Integer;
begin
  Result := 17 + 4 * VersionNumber;
end;

function TVersion.GetECBlocksForLevel(ECLevel: TErrorCorrectionLevel): TECBlocks;
begin
  Result := ECBlocks[ECLevel.Ordinal];
end;

function TVersion.GetTotalCodewords: Integer;
begin
  Result := TotalCodewords;
end;

class function TVersion.GetVersionForNumber(VersionNum: Integer): TVersion;
begin
  if (VersionNum = 1) then
  begin
    Result := TVersion.Create(1, [],
      TECBlocks.Create(7, TECB.Create(1, 19)),
      TECBlocks.Create(10, TECB.Create(1, 16)),
      TECBlocks.Create(13, TECB.Create(1, 13)),
      TECBlocks.Create(17, TECB.Create(1, 9)));
  end else
  if (VersionNum = 2) then
  begin
    Result := TVersion.Create(2, [6, 18],
      TECBlocks.Create(10, TECB.Create(1, 34)),
      TECBlocks.Create(16, TECB.Create(1, 28)),
      TECBlocks.Create(22, TECB.Create(1, 22)),
      TECBlocks.Create(28, TECB.Create(1, 16)));
  end else
  if (VersionNum = 3) then
  begin
    Result := TVersion.Create(3, [6, 22],
      TECBlocks.Create(15, TECB.Create(1, 55)),
      TECBlocks.Create(26, TECB.Create(1, 44)),
      TECBlocks.Create(18, TECB.Create(2, 17)),
      TECBlocks.Create(22, TECB.Create(2, 13)));
  end else
  if (VersionNum = 4) then
  begin
    Result := TVersion.Create(4, [6, 26],
      TECBlocks.Create(20, TECB.Create(1, 80)),
      TECBlocks.Create(18, TECB.Create(2, 32)),
      TECBlocks.Create(26, TECB.Create(2, 24)),
      TECBlocks.Create(16, TECB.Create(4, 9)));
  end else
  if (VersionNum = 5) then
  begin
    Result := TVersion.Create(5, [6, 30],
      TECBlocks.Create(26, TECB.Create(1, 108)),
      TECBlocks.Create(24, TECB.Create(2, 43)),
      TECBlocks.Create(18, TECB.Create(2, 15),
          TECB.Create(2, 16)),
      TECBlocks.Create(22, TECB.Create(2, 11),
          TECB.Create(2, 12)));
  end else
  if (VersionNum = 6) then
  begin
    Result := TVersion.Create(6, [6, 34],
      TECBlocks.Create(18, TECB.Create(2, 68)),
      TECBlocks.Create(16, TECB.Create(4, 27)),
      TECBlocks.Create(24, TECB.Create(4, 19)),
      TECBlocks.Create(28, TECB.Create(4, 15)));
  end else
  if (VersionNum = 7) then
  begin
    Result := TVersion.Create(7, [6, 22, 38],
      TECBlocks.Create(20, TECB.Create(2, 78)),
      TECBlocks.Create(18, TECB.Create(4, 31)),
      TECBlocks.Create(18, TECB.Create(2, 14),
          TECB.Create(4, 15)),
      TECBlocks.Create(26, TECB.Create(4, 13),
          TECB.Create(1, 14)));
  end else
  if (VersionNum = 8) then
  begin
    Result := TVersion.Create(8, [6, 24, 42],
      TECBlocks.Create(24, TECB.Create(2, 97)),
      TECBlocks.Create(22, TECB.Create(2, 38),
          TECB.Create(2, 39)),
      TECBlocks.Create(22, TECB.Create(4, 18),
          TECB.Create(2, 19)),
      TECBlocks.Create(26, TECB.Create(4, 14),
          TECB.Create(2, 15)));
  end else
  if (VersionNum = 9) then
  begin
    Result := TVersion.Create(9, [6, 26, 46],
      TECBlocks.Create(30, TECB.Create(2, 116)),
      TECBlocks.Create(22, TECB.Create(3, 36),
          TECB.Create(2, 37)),
      TECBlocks.Create(20, TECB.Create(4, 16),
          TECB.Create(4, 17)),
      TECBlocks.Create(24, TECB.Create(4, 12),
          TECB.Create(4, 13)));
  end else
  if (VersionNum = 10) then
  begin
    Result := TVersion.Create(10, [6, 28, 50],
      TECBlocks.Create(18, TECB.Create(2, 68),
          TECB.Create(2, 69)),
      TECBlocks.Create(26, TECB.Create(4, 43),
          TECB.Create(1, 44)),
      TECBlocks.Create(24, TECB.Create(6, 19),
          TECB.Create(2, 20)),
      TECBlocks.Create(28, TECB.Create(6, 15),
          TECB.Create(2, 16)));
  end else
  if (VersionNum = 11) then
  begin
    Result := TVersion.Create(11, [6, 30, 54],
      TECBlocks.Create(20, TECB.Create(4, 81)),
      TECBlocks.Create(30, TECB.Create(1, 50),
          TECB.Create(4, 51)),
      TECBlocks.Create(28, TECB.Create(4, 22),
          TECB.Create(4, 23)),
      TECBlocks.Create(24, TECB.Create(3, 12),
          TECB.Create(8, 13)));
  end else
  if (VersionNum = 12) then
  begin
    Result := TVersion.Create(12, [6, 32, 58],
      TECBlocks.Create(24, TECB.Create(2, 92),
          TECB.Create(2, 93)),
      TECBlocks.Create(22, TECB.Create(6, 36),
          TECB.Create(2, 37)),
      TECBlocks.Create(26, TECB.Create(4, 20),
          TECB.Create(6, 21)),
      TECBlocks.Create(28, TECB.Create(7, 14),
          TECB.Create(4, 15)));
  end else
  if (VersionNum = 13) then
  begin
    Result := TVersion.Create(13, [6, 34, 62],
      TECBlocks.Create(26, TECB.Create(4, 107)),
      TECBlocks.Create(22, TECB.Create(8, 37),
          TECB.Create(1, 38)),
      TECBlocks.Create(24, TECB.Create(8, 20),
          TECB.Create(4, 21)),
      TECBlocks.Create(22, TECB.Create(12, 11),
          TECB.Create(4, 12)));
  end else
  if (VersionNum = 14) then
  begin
    Result := TVersion.Create(14, [6, 26, 46, 66],
      TECBlocks.Create(30, TECB.Create(3, 115),
          TECB.Create(1, 116)),
      TECBlocks.Create(24, TECB.Create(4, 40),
          TECB.Create(5, 41)),
      TECBlocks.Create(20, TECB.Create(11, 16),
          TECB.Create(5, 17)),
      TECBlocks.Create(24, TECB.Create(11, 12),
          TECB.Create(5, 13)));
  end else
  if (VersionNum = 15) then
  begin
    Result := TVersion.Create(15, [6, 26, 48, 70],
      TECBlocks.Create(22, TECB.Create(5, 87),
          TECB.Create(1, 88)),
      TECBlocks.Create(24, TECB.Create(5, 41),
          TECB.Create(5, 42)),
      TECBlocks.Create(30, TECB.Create(5, 24),
          TECB.Create(7, 25)),
      TECBlocks.Create(24, TECB.Create(11, 12),
          TECB.Create(7, 13)));
  end else
  if (VersionNum = 16) then
  begin
    Result := TVersion.Create(16, [6, 26, 50, 74],
      TECBlocks.Create(24, TECB.Create(5, 98),
          TECB.Create(1, 99)),
      TECBlocks.Create(28, TECB.Create(7, 45),
          TECB.Create(3, 46)),
      TECBlocks.Create(24, TECB.Create(15, 19),
          TECB.Create(2, 20)),
      TECBlocks.Create(30, TECB.Create(3, 15),
          TECB.Create(13, 16)));
  end else
  if (VersionNum = 17) then
  begin
    Result := TVersion.Create(17, [6, 30, 54, 78],
      TECBlocks.Create(28, TECB.Create(1, 107),
          TECB.Create(5, 108)),
      TECBlocks.Create(28, TECB.Create(10, 46),
          TECB.Create(1, 47)),
      TECBlocks.Create(28, TECB.Create(1, 22),
          TECB.Create(15, 23)),
      TECBlocks.Create(28, TECB.Create(2, 14),
          TECB.Create(17, 15)));
  end else
  if (VersionNum = 18) then
  begin
    Result := TVersion.Create(18, [6, 30, 56, 82],
      TECBlocks.Create(30, TECB.Create(5, 120),
          TECB.Create(1, 121)),
      TECBlocks.Create(26, TECB.Create(9, 43),
          TECB.Create(4, 44)),
      TECBlocks.Create(28, TECB.Create(17, 22),
          TECB.Create(1, 23)),
      TECBlocks.Create(28, TECB.Create(2, 14),
          TECB.Create(19, 15)));
  end else
  if (VersionNum = 19) then
  begin
    Result := TVersion.Create(19, [6, 30, 58, 86],
      TECBlocks.Create(28, TECB.Create(3, 113),
          TECB.Create(4, 114)),
      TECBlocks.Create(26, TECB.Create(3, 44),
          TECB.Create(11, 45)),
      TECBlocks.Create(26, TECB.Create(17, 21),
          TECB.Create(4, 22)),
      TECBlocks.Create(26, TECB.Create(9, 13),
          TECB.Create(16, 14)));
  end else
  if (VersionNum = 20) then
  begin
    Result := TVersion.Create(20, [6, 34, 62, 90],
      TECBlocks.Create(28, TECB.Create(3, 107),
          TECB.Create(5, 108)),
      TECBlocks.Create(26, TECB.Create(3, 41),
          TECB.Create(13, 42)),
      TECBlocks.Create(30, TECB.Create(15, 24),
          TECB.Create(5, 25)),
      TECBlocks.Create(28, TECB.Create(15, 15),
          TECB.Create(10, 16)));
  end else
  if (VersionNum = 21) then
  begin
    Result := TVersion.Create(21, [6, 28, 50, 72, 94],
      TECBlocks.Create(28, TECB.Create(4, 116),
          TECB.Create(4, 117)),
      TECBlocks.Create(26, TECB.Create(17, 42)),
      TECBlocks.Create(28, TECB.Create(17, 22),
          TECB.Create(6, 23)),
      TECBlocks.Create(30, TECB.Create(19, 16),
          TECB.Create(6, 17)));
  end else
  if (VersionNum = 22) then
  begin
    Result := TVersion.Create(22, [6, 26, 50, 74, 98],
      TECBlocks.Create(28, TECB.Create(2, 111),
          TECB.Create(7, 112)),
      TECBlocks.Create(28, TECB.Create(17, 46)),
      TECBlocks.Create(30, TECB.Create(7, 24),
          TECB.Create(16, 25)),
      TECBlocks.Create(24, TECB.Create(34, 13)));
  end else
  if (VersionNum = 23) then
  begin
    Result := TVersion.Create(23, [6, 30, 54, 78, 102],
      TECBlocks.Create(30, TECB.Create(4, 121),
          TECB.Create(5, 122)),
      TECBlocks.Create(28, TECB.Create(4, 47),
          TECB.Create(14, 48)),
      TECBlocks.Create(30, TECB.Create(11, 24),
          TECB.Create(14, 25)),
      TECBlocks.Create(30, TECB.Create(16, 15),
          TECB.Create(14, 16)));
  end else
  if (VersionNum = 24) then
  begin
    Result := TVersion.Create(24, [6, 28, 54, 80, 106],
      TECBlocks.Create(30, TECB.Create(6, 117),
          TECB.Create(4, 118)),
      TECBlocks.Create(28, TECB.Create(6, 45),
          TECB.Create(14, 46)),
      TECBlocks.Create(30, TECB.Create(11, 24),
          TECB.Create(16, 25)),
      TECBlocks.Create(30, TECB.Create(30, 16),
          TECB.Create(2, 17)));
  end else
  if (VersionNum = 25) then
  begin
    Result := TVersion.Create(25, [6, 32, 58, 84, 110],
      TECBlocks.Create(26, TECB.Create(8, 106),
          TECB.Create(4, 107)),
      TECBlocks.Create(28, TECB.Create(8, 47),
          TECB.Create(13, 48)),
      TECBlocks.Create(30, TECB.Create(7, 24),
          TECB.Create(22, 25)),
      TECBlocks.Create(30, TECB.Create(22, 15),
          TECB.Create(13, 16)));
  end else
  if (VersionNum = 26) then
  begin
    Result := TVersion.Create(26, [6, 30, 58, 86, 114],
      TECBlocks.Create(28, TECB.Create(10, 114),
          TECB.Create(2, 115)),
      TECBlocks.Create(28, TECB.Create(19, 46),
          TECB.Create(4, 47)),
      TECBlocks.Create(28, TECB.Create(28, 22),
          TECB.Create(6, 23)),
      TECBlocks.Create(30, TECB.Create(33, 16),
          TECB.Create(4, 17)));
  end else
  if (VersionNum = 27) then
  begin
    Result := TVersion.Create(27, [6, 34, 62, 90, 118],
      TECBlocks.Create(30, TECB.Create(8, 122),
          TECB.Create(4, 123)),
      TECBlocks.Create(28, TECB.Create(22, 45),
          TECB.Create(3, 46)),
      TECBlocks.Create(30, TECB.Create(8, 23),
          TECB.Create(26, 24)),
      TECBlocks.Create(30, TECB.Create(12, 15),
          TECB.Create(28, 16)));
  end else
  if (VersionNum = 28) then
  begin
    Result := TVersion.Create(28, [6, 26, 50, 74, 98, 122],
      TECBlocks.Create(30, TECB.Create(3, 117),
          TECB.Create(10, 118)),
      TECBlocks.Create(28, TECB.Create(3, 45),
          TECB.Create(23, 46)),
      TECBlocks.Create(30, TECB.Create(4, 24),
          TECB.Create(31, 25)),
      TECBlocks.Create(30, TECB.Create(11, 15),
          TECB.Create(31, 16)));
  end else
  if (VersionNum = 29) then
  begin
    Result := TVersion.Create(29, [6, 30, 54, 78, 102, 126],
      TECBlocks.Create(30, TECB.Create(7, 116),
          TECB.Create(7, 117)),
      TECBlocks.Create(28, TECB.Create(21, 45),
          TECB.Create(7, 46)),
      TECBlocks.Create(30, TECB.Create(1, 23),
          TECB.Create(37, 24)),
      TECBlocks.Create(30, TECB.Create(19, 15),
          TECB.Create(26, 16)));
  end else
  if (VersionNum = 30) then
  begin
    Result := TVersion.Create(30, [6, 26, 52, 78, 104, 130],
      TECBlocks.Create(30, TECB.Create(5, 115),
          TECB.Create(10, 116)),
      TECBlocks.Create(28, TECB.Create(19, 47),
          TECB.Create(10, 48)),
      TECBlocks.Create(30, TECB.Create(15, 24),
          TECB.Create(25, 25)),
      TECBlocks.Create(30, TECB.Create(23, 15),
          TECB.Create(25, 16)));
  end else
  if (VersionNum = 31) then
  begin
    Result := TVersion.Create(31, [6, 30, 56, 82, 108, 134],
      TECBlocks.Create(30, TECB.Create(13, 115),
          TECB.Create(3, 116)),
      TECBlocks.Create(28, TECB.Create(2, 46),
          TECB.Create(29, 47)),
      TECBlocks.Create(30, TECB.Create(42, 24),
          TECB.Create(1, 25)),
      TECBlocks.Create(30, TECB.Create(23, 15),
          TECB.Create(28, 16)));
  end else
  if (VersionNum = 32) then
  begin
    Result := TVersion.Create(32, [6, 34, 60, 86, 112, 138],
      TECBlocks.Create(30, TECB.Create(17, 115)),
      TECBlocks.Create(28, TECB.Create(10, 46),
          TECB.Create(23, 47)),
      TECBlocks.Create(30, TECB.Create(10, 24),
          TECB.Create(35, 25)),
      TECBlocks.Create(30, TECB.Create(19, 15),
          TECB.Create(35, 16)));
  end else
  if (VersionNum = 33) then
  begin
    Result := TVersion.Create(33, [6, 30, 58, 86, 114, 142],
      TECBlocks.Create(30, TECB.Create(17, 115),
          TECB.Create(1, 116)),
      TECBlocks.Create(28, TECB.Create(14, 46),
          TECB.Create(21, 47)),
      TECBlocks.Create(30, TECB.Create(29, 24),
          TECB.Create(19, 25)),
      TECBlocks.Create(30, TECB.Create(11, 15),
          TECB.Create(46, 16)));
  end else
  if (VersionNum = 34) then
  begin
    Result := TVersion.Create(34, [6, 34, 62, 90, 118, 146],
      TECBlocks.Create(30, TECB.Create(13, 115),
          TECB.Create(6, 116)),
      TECBlocks.Create(28, TECB.Create(14, 46),
          TECB.Create(23, 47)),
      TECBlocks.Create(30, TECB.Create(44, 24),
          TECB.Create(7, 25)),
      TECBlocks.Create(30, TECB.Create(59, 16),
          TECB.Create(1, 17)));
  end else
  if (VersionNum = 35) then
  begin
    Result := TVersion.Create(35, [6, 30, 54, 78, 102, 126, 150],
      TECBlocks.Create(30, TECB.Create(12, 121),
          TECB.Create(7, 122)),
      TECBlocks.Create(28, TECB.Create(12, 47),
          TECB.Create(26, 48)),
      TECBlocks.Create(30, TECB.Create(39, 24),
          TECB.Create(14, 25)),
      TECBlocks.Create(30, TECB.Create(22, 15),
          TECB.Create(41, 16)));
  end else
  if (VersionNum = 36) then
  begin
    Result := TVersion.Create(36, [6, 24, 50, 76, 102, 128, 154],
      TECBlocks.Create(30, TECB.Create(6, 121),
          TECB.Create(14, 122)),
      TECBlocks.Create(28, TECB.Create(6, 47),
          TECB.Create(34, 48)),
      TECBlocks.Create(30, TECB.Create(46, 24),
          TECB.Create(10, 25)),
      TECBlocks.Create(30, TECB.Create(2, 15),
          TECB.Create(64, 16)));
  end else
  if (VersionNum = 37) then
  begin
    Result := TVersion.Create(37, [6, 28, 54, 80, 106, 132, 158],
      TECBlocks.Create(30, TECB.Create(17, 122),
          TECB.Create(4, 123)),
      TECBlocks.Create(28, TECB.Create(29, 46),
          TECB.Create(14, 47)),
      TECBlocks.Create(30, TECB.Create(49, 24),
          TECB.Create(10, 25)),
      TECBlocks.Create(30, TECB.Create(24, 15),
          TECB.Create(46, 16)));
  end else
  if (VersionNum = 38) then
  begin
    Result := TVersion.Create(38, [6, 32, 58, 84, 110, 136, 162],
      TECBlocks.Create(30, TECB.Create(4, 122),
          TECB.Create(18, 123)),
      TECBlocks.Create(28, TECB.Create(13, 46),
          TECB.Create(32, 47)),
      TECBlocks.Create(30, TECB.Create(48, 24),
          TECB.Create(14, 25)),
      TECBlocks.Create(30, TECB.Create(42, 15),
          TECB.Create(32, 16)));
  end else
  if (VersionNum = 39) then
  begin
    Result := TVersion.Create(39, [6, 26, 54, 82, 110, 138, 166],
      TECBlocks.Create(30, TECB.Create(20, 117),
          TECB.Create(4, 118)),
      TECBlocks.Create(28, TECB.Create(40, 47),
          TECB.Create(7, 48)),
      TECBlocks.Create(30, TECB.Create(43, 24),
          TECB.Create(22, 25)),
      TECBlocks.Create(30, TECB.Create(10, 15),
          TECB.Create(67, 16)));
  end else
  if (VersionNum = 40) then
  begin
    Result := TVersion.Create(40, [6, 30, 58, 86, 114, 142, 170],
      TECBlocks.Create(30, TECB.Create(19, 118),
          TECB.Create(6, 119)),
      TECBlocks.Create(28, TECB.Create(18, 47),
          TECB.Create(31, 48)),
      TECBlocks.Create(30, TECB.Create(34, 24),
          TECB.Create(34, 25)),
      TECBlocks.Create(30, TECB.Create(20, 15),
          TECB.Create(61, 16)));
  end else
  begin
    Result := nil;
  end;
end;

{ TMaskUtil }

// Return the mask bit for "getMaskPattern" at "x" and "y". See 8.8 of JISX0510:2004 for mask
// pattern conditions.
function TMaskUtil.GetDataMaskBit(MaskPattern, X, Y: Integer): Boolean;
var
  Intermediate: Integer;
  Temp: Integer;
begin
  Intermediate := 0;
  if ((MaskPattern >= 0) and (MaskPattern < NUM_MASK_PATTERNS)) then
  begin
    case (maskPattern) of
      0: Intermediate := (Y + X) and 1;
      1: Intermediate := Y and 1;
      2: Intermediate := X mod 3;
      3: Intermediate := (Y + X) mod 3;
      4: Intermediate := ((y shr 1) + (X div 3)) and 1;
      5:
      begin
        Temp := Y * X;
        Intermediate := (Temp and 1) + (Temp mod 3);
      end;
      6:
      begin
        Temp := Y * X;
        Intermediate := ((Temp and 1) + (Temp mod 3)) and 1;
      end;
      7:
      begin
        Temp := Y * X;
        Intermediate := ((temp mod 3) + ((Y + X) and 1)) and 1;
      end;
    end;
  end;
  Result := Intermediate = 0;
end;

{ TECBlocks }

constructor TECBlocks.Create(ECCodewordsPerBlock: Integer; ECBlocks: TECB);
begin
  Self.ECCodewordsPerBlock := ECCodewordsPerBlock;
  SetLength(Self.ECBlocks, 1);
  Self.ECBlocks[0] := ECBlocks;
end;

constructor TECBlocks.Create(ECCodewordsPerBlock: Integer; ECBlocks1,
  ECBlocks2: TECB);
begin
  Self.ECCodewordsPerBlock := ECCodewordsPerBlock;
  SetLength(Self.ECBlocks, 2);
  ECBlocks[0] := ECBlocks1;
  ECBlocks[1] := ECBlocks2;
end;

destructor TECBlocks.Destroy;
var
  X: Integer;
begin
  for X := 0 to Length(ECBlocks) - 1 do
  begin
    ECBlocks[X].Free;
  end;
  inherited;
end;

function TECBlocks.GetECBlocks: TECBArray;
begin
  Result := ECBlocks;
end;

function TECBlocks.GetECCodewordsPerBlock: Integer;
begin
  Result := ECCodewordsPerBlock;
end;

function TECBlocks.GetNumBlocks: Integer;
var
  Total: Integer;
  I: Integer;
begin
  Total := 0;
  for I := 0 to Length(ECBlocks) - 1 do
  begin
    Inc(Total, ECBlocks[I].GetCount);
  end;
  Result := Total;
end;

function TECBlocks.GetTotalECCodewords: Integer;
begin
  Result := ECCodewordsPerBlock * GetNumBlocks;
end;

{ TBlockPair }

constructor TBlockPair.Create(BA1, BA2: TByteArray);
begin
  FDataBytes := BA1;
  FErrorCorrectionBytes := BA2;
end;

function TBlockPair.GetDataBytes: TByteArray;
begin
  Result := FDataBytes;
end;

function TBlockPair.GetErrorCorrectionBytes: TByteArray;
begin
  Result := FErrorCorrectionBytes;
end;

{ TReedSolomonEncoder }

function TReedSolomonEncoder.BuildGenerator(Degree: Integer): TGenericGFPoly;
var
  LastGenerator: TGenericGFPoly;
  NextGenerator: TGenericGFPoly;
  Poly: TGenericGFPoly;
  D: Integer;
  CA: TIntegerArray;
begin
  if (Degree >= FCachedGenerators.Count) then
  begin
    LastGenerator := TGenericGFPoly(FCachedGenerators[FCachedGenerators.Count - 1]);

    for D := FCachedGenerators.Count to Degree do
    begin
      SetLength(CA, 2);
      CA[0] := 1;
      CA[1] := FField.Exp(D - 1 + FField.GetGeneratorBase);
      Poly := TGenericGFPoly.Create(FField, CA);
      NextGenerator := LastGenerator.Multiply(Poly);
      FCachedGenerators.Add(NextGenerator);
      LastGenerator := NextGenerator;
    end;
  end;
  Result := TGenericGFPoly(FCachedGenerators[Degree]);
end;

constructor TReedSolomonEncoder.Create(AField: TGenericGF);
var
  GenericGFPoly: TGenericGFPoly;
  IntArray: TIntegerArray;
begin
  FField := AField;

  // Contents of FCachedGenerators will be freed by FGenericGF.Destroy
  FCachedGenerators := TObjectList.Create(False);

  SetLength(IntArray, 1);
  IntArray[0] := 1;
  GenericGFPoly := TGenericGFPoly.Create(AField, IntArray);
  FCachedGenerators.Add(GenericGFPoly);
end;

destructor TReedSolomonEncoder.Destroy;
begin
  FCachedGenerators.Free;
  inherited;
end;

procedure TReedSolomonEncoder.Encode(ToEncode: TIntegerArray; ECBytes: Integer);
var
  DataBytes: Integer;
  Generator: TGenericGFPoly;
  InfoCoefficients: TIntegerArray;
  Info: TGenericGFPoly;
  Remainder: TGenericGFPoly;
  Coefficients: TIntegerArray;
  NumZeroCoefficients: Integer;
  I: Integer;
begin
  SetLength(Coefficients, 0);
  if (ECBytes > 0) then
  begin
    DataBytes := Length(ToEncode) - ECBytes;
    if (DataBytes > 0) then
    begin
      Generator := BuildGenerator(ECBytes);
      SetLength(InfoCoefficients, DataBytes);
      InfoCoefficients := Copy(ToEncode, 0, DataBytes);
      Info := TGenericGFPoly.Create(FField, InfoCoefficients);
      Info := Info.MultiplyByMonomial(ECBytes, 1);
      Remainder := Info.Divide(Generator)[1];
      Coefficients := Remainder.GetCoefficients;
      NumZeroCoefficients := ECBytes - Length(Coefficients);
      for I := 0 to NumZeroCoefficients - 1 do
      begin
        ToEncode[DataBytes + I] := 0;
      end;
      Move(Coefficients[0], ToEncode[DataBytes + NumZeroCoefficients], Length(Coefficients) * SizeOf(Integer));
    end;
  end;
end;

{ TECB }

constructor TECB.Create(Count, DataCodewords: Integer);
begin
  Self.Count := Count;
  Self.DataCodewords := DataCodewords;
end;

function TECB.GetCount: Integer;
begin
  Result := Count;
end;

function TECB.GetDataCodewords: Integer;
begin
  Result := DataCodewords;
end;

{ TGenericGFPoly }

function TGenericGFPoly.AddOrSubtract(Other: TGenericGFPoly): TGenericGFPoly;
var
  SmallerCoefficients: TIntegerArray;
  LargerCoefficients: TIntegerArray;
  Temp: TIntegerArray;
  SumDiff: TIntegerArray;
  LengthDiff: Integer;
  I: Integer;
begin
  SetLength(SmallerCoefficients, 0);
  SetLength(LargerCoefficients, 0);
  SetLength(Temp, 0);
  SetLength(SumDiff, 0);

  Result := nil;
  if (Assigned(Other)) then
  begin
    if (FField = Other.FField) then
    begin
      if (IsZero) then
      begin
        Result := Other;
        Exit;
      end;

      if (Other.IsZero) then
      begin
        Result := Self;
        Exit;
      end;

      SmallerCoefficients := FCoefficients;
      LargerCoefficients := Other.Coefficients;
      if (Length(SmallerCoefficients) > Length(LargerCoefficients)) then
      begin
        Temp := smallerCoefficients;
        SmallerCoefficients := LargerCoefficients;
        LargerCoefficients := temp;
      end;
      SetLength(SumDiff, Length(LargerCoefficients));
      LengthDiff := Length(LargerCoefficients) - Length(SmallerCoefficients);

      // Copy high-order terms only found in higher-degree polynomial's coefficients
      if (LengthDiff > 0) then
      begin
        //SumDiff := Copy(LargerCoefficients, 0, LengthDiff);
        Move(LargerCoefficients[0], SumDiff[0], LengthDiff * SizeOf(Integer));
      end;

      for I := LengthDiff to Length(LargerCoefficients) - 1 do
      begin
        SumDiff[I] := TGenericGF.AddOrSubtract(SmallerCoefficients[I - LengthDiff], LargerCoefficients[I]);
      end;

      Result := TGenericGFPoly.Create(FField, SumDiff);
    end;
  end;
end;

function TGenericGFPoly.Coefficients: TIntegerArray;
begin
  Result := FCoefficients;
end;

constructor TGenericGFPoly.Create(AField: TGenericGF;
  ACoefficients: TIntegerArray);
var
  CoefficientsLength: Integer;
  FirstNonZero: Integer;
begin
  FField := AField;
  SetLength(FField.FPolyList, Length(FField.FPolyList) + 1);
  FField.FPolyList[Length(FField.FPolyList) - 1] := Self;
  CoefficientsLength := Length(ACoefficients);
  if ((CoefficientsLength > 1) and (ACoefficients[0] = 0)) then
  begin
    // Leading term must be non-zero for anything except the constant polynomial "0"
    FirstNonZero := 1;
    while ((FirstNonZero < CoefficientsLength) and (ACoefficients[FirstNonZero] = 0)) do
    begin
      Inc(FirstNonZero);
    end;

    if (FirstNonZero = CoefficientsLength) then
    begin
      FCoefficients := AField.GetZero.Coefficients;
    end else
    begin
      SetLength(FCoefficients, CoefficientsLength - FirstNonZero);
      FCoefficients := Copy(ACoefficients, FirstNonZero, Length(FCoefficients));
    end;
  end else
  begin
    FCoefficients := ACoefficients;
  end;
end;

destructor TGenericGFPoly.Destroy;
begin
  Self.FField := FField;
  inherited;
end;

function TGenericGFPoly.Divide(Other: TGenericGFPoly): TGenericGFPolyArray;
var
  Quotient: TGenericGFPoly;
  Remainder: TGenericGFPoly;
  DenominatorLeadingTerm: Integer;
  InverseDenominatorLeadingTerm: integer;
  DegreeDifference: Integer;
  Scale: Integer;
  Term: TGenericGFPoly;
  IterationQuotient: TGenericGFPoly;
begin
  SetLength(Result, 0);
  if ((FField = Other.FField) and (not Other.IsZero)) then
  begin

    Quotient := FField.GetZero;
    Remainder := Self;

    DenominatorLeadingTerm := Other.GetCoefficient(Other.GetDegree);
    InverseDenominatorLeadingTerm := FField.Inverse(DenominatorLeadingTerm);

    while ((Remainder.GetDegree >= Other.GetDegree) and (not Remainder.IsZero)) do
    begin
      DegreeDifference := Remainder.GetDegree - Other.GetDegree;
      Scale := FField.Multiply(Remainder.GetCoefficient(Remainder.GetDegree), InverseDenominatorLeadingTerm);
      Term := Other.MultiplyByMonomial(DegreeDifference, Scale);
      IterationQuotient := FField.BuildMonomial(degreeDifference, scale);
      Quotient := Quotient.AddOrSubtract(IterationQuotient);
      Remainder := Remainder.AddOrSubtract(Term);
    end;

    SetLength(Result, 2);
    Result[0] := Quotient;
    Result[1] := Remainder;
  end;
end;

function TGenericGFPoly.GetCoefficient(Degree: Integer): Integer;
begin
  Result := FCoefficients[Length(FCoefficients) - 1 - Degree];
end;

function TGenericGFPoly.GetCoefficients: TIntegerArray;
begin
  Result := FCoefficients;
end;

function TGenericGFPoly.GetDegree: Integer;
begin
  Result := Length(FCoefficients) - 1;
end;

function TGenericGFPoly.IsZero: Boolean;
begin
  Result := FCoefficients[0] = 0;
end;

function TGenericGFPoly.Multiply(Other: TGenericGFPoly): TGenericGFPoly;
var
  ACoefficients: TIntegerArray;
  BCoefficients: TIntegerArray;
  Product: TIntegerArray;
  ALength: Integer;
  BLength: Integer;
  I: Integer;
  J: Integer;
  ACoeff: Integer;
begin
  SetLength(ACoefficients, 0);
  SetLength(BCoefficients, 0);
  Result := nil;

  if (FField = Other.FField) then
  begin
    if (IsZero or Other.IsZero) then
    begin
      Result := FField.GetZero;
      Exit;
    end;

    ACoefficients := FCoefficients;
    ALength := Length(ACoefficients);
    BCoefficients := Other.Coefficients;
    BLength := Length(BCoefficients);
    SetLength(Product, aLength + bLength - 1);
    for I := 0 to ALength - 1 do
    begin
      ACoeff := ACoefficients[I];
      for J := 0 to BLength - 1 do
      begin
        Product[I + J] := TGenericGF.AddOrSubtract(Product[I + J],
          FField.Multiply(ACoeff, BCoefficients[J]));
      end;
    end;
    Result := TGenericGFPoly.Create(FField, Product);
  end;
end;

function TGenericGFPoly.MultiplyByMonomial(Degree,
  Coefficient: Integer): TGenericGFPoly;
var
  I: Integer;
  Size: Integer;
  Product: TIntegerArray;
begin
  Result := nil;
  if (Degree >= 0) then
  begin
    if (Coefficient = 0) then
    begin
      Result := FField.GetZero;
      Exit;
    end;
    Size := Length(Coefficients);
    SetLength(Product, Size + Degree);
    for I := 0 to Size - 1 do
    begin
      Product[I] := FField.Multiply(FCoefficients[I], Coefficient);
    end;
    Result := TGenericGFPoly.Create(FField, Product);
  end;
end;

{ TGenericGF }

class function TGenericGF.AddOrSubtract(A, B: Integer): Integer;
begin
  Result := A xor B;
end;

function TGenericGF.BuildMonomial(Degree, Coefficient: Integer): TGenericGFPoly;
var
  Coefficients: TIntegerArray;
begin
  CheckInit();

  if (Degree >= 0) then
  begin
    if (Coefficient = 0) then
    begin
      Result := FZero;
      Exit;
    end;
    SetLength(Coefficients, Degree + 1);
    Coefficients[0] := Coefficient;
    Result := TGenericGFPoly.Create(Self, Coefficients);
  end else
  begin
    Result := nil;
  end;
end;

procedure TGenericGF.CheckInit;
begin
  if (not FInitialized) then
  begin
    Initialize;
  end;
end;

constructor TGenericGF.Create(Primitive, Size, B: Integer);
begin
  FInitialized := False;
  FPrimitive := Primitive;
  FSize := Size;
  FGeneratorBase := B;
  if (FSize < 0) then
  begin
    Initialize;
  end;
end;

class function TGenericGF.CreateQRCodeField256: TGenericGF;
begin
  Result := TGenericGF.Create($011D, 256, 0);
end;

destructor TGenericGF.Destroy;
var
  X: Integer;
  Y: Integer;
begin
  for X := 0 to Length(FPolyList) - 1 do
  begin
    if (Assigned(FPolyList[X])) then
    begin
      for Y := X + 1 to Length(FPolyList) - 1 do
      begin
        if (FPolyList[Y] = FPolyList[X]) then
        begin
          FPolyList[Y] := nil;
        end;
      end;
      FPolyList[X].Free;
    end;
  end;
  inherited;
end;

function TGenericGF.Exp(A: Integer): Integer;
begin
  CheckInit;
  Result := FExpTable[A];
end;

function TGenericGF.GetGeneratorBase: Integer;
begin
  Result := FGeneratorBase;
end;

function TGenericGF.GetZero: TGenericGFPoly;
begin
  CheckInit;
  Result := FZero;
end;

procedure TGenericGF.Initialize;
var
  X: Integer;
  I: Integer;
  CA: TIntegerArray;
begin
  SetLength(FExpTable, FSize);
  SetLength(FLogTable, FSize);
  X := 1;
  for I := 0 to FSize - 1 do
  begin
    FExpTable[I] := x;
    X := X shl 1; // x = x * 2; we're assuming the generator alpha is 2
    if (X >= FSize) then
    begin
        X := X xor FPrimitive;
        X := X and (FSize - 1);
    end;
  end;

  for I := 0 to FSize - 2 do
  begin
    FLogTable[FExpTable[I]] := I;
  end;

  // logTable[0] == 0 but this should never be used

  SetLength(CA, 1);
  CA[0] := 0;
  FZero := TGenericGFPoly.Create(Self, CA);

  SetLength(CA, 1);
  CA[0] := 1;
  FOne := TGenericGFPoly.Create(Self, CA);

  FInitialized := True;
end;

function TGenericGF.Inverse(A: Integer): Integer;
begin
  CheckInit;

  if (a <> 0) then
  begin
    Result := FExpTable[FSize - FLogTable[A] - 1];
  end else
  begin
    Result := 0;
  end;
end;

function TGenericGF.Multiply(A, B: Integer): Integer;
begin
  CheckInit;
  if ((A <> 0) and (B <> 0)) then
  begin
    Result := FExpTable[(FLogTable[A] + FLogTable[B]) mod (FSize - 1)];
  end else
  begin
    Result := 0;
  end;
end;

function GenerateQRCode(const Input: WideString; EncodeOptions: Integer): T2DBooleanArray;
var
  Encoder: TEncoder;
  Level: TErrorCorrectionLevel;
  QRCode: TQRCode;
  X: Integer;
  Y: Integer;
begin
  Level := TErrorCorrectionLevel.Create;
  Level.FBits := 1;
  Encoder := TEncoder.Create;
  QRCode := TQRCode.Create;
  try
    Encoder.Encode(Input, EncodeOptions, Level, QRCode);
    if (Assigned(QRCode.FMatrix)) then
    begin
      SetLength(Result, QRCode.FMatrix.FHeight);
      for Y := 0 to QRCode.FMatrix.FHeight - 1 do
      begin
        SetLength(Result[Y], QRCode.FMatrix.FWidth);
        for X := 0 to QRCode.FMatrix.FWidth - 1 do
        begin
          Result[Y][X] := QRCode.FMatrix.Get(Y, X) = 1;
        end;
      end;
    end;
  finally
    QRCode.Free;
    Encoder.Free;
    Level.Free;
  end;
end;

{ TDelphiZXingQRCode }

constructor TDelphiZXingQRCode.Create;
begin
  FData := '';
  FEncoding := qrAuto;
  FQuietZone := 4;
  FRows := 0;
  FColumns := 0;
end;

function TDelphiZXingQRCode.GetIsBlack(Row, Column: Integer): Boolean;
begin
  Dec(Row, FQuietZone);
  Dec(Column, FQuietZone);
  if ((Row >= 0) and (Column >= 0) and (Row < (FRows - FQuietZone * 2)) and (Column < (FColumns - FQuietZone * 2))) then
  begin
    Result := FElements[Column, Row];
  end else
  begin
    Result := False;
  end;
end;

procedure TDelphiZXingQRCode.SetData(const NewData: WideString);
begin
  if (FData <> NewData) then
  begin
    FData := NewData;
    Update;
  end;
end;

procedure TDelphiZXingQRCode.SetEncoding(NewEncoding: TQRCodeEncoding);
begin
  if (FEncoding <> NewEncoding) then
  begin
    FEncoding := NewEncoding;
    Update;
  end;
end;

procedure TDelphiZXingQRCode.SetQuietZone(NewQuietZone: Integer);
begin
  if ((FQuietZone <> NewQuietZone) and (NewQuietZone >= 0) and (NewQuietZone <= 100)) then
  begin
    FQuietZone := NewQuietZone;
    Update;
  end;
end;

procedure TDelphiZXingQRCode.Update;
begin
  FElements := GenerateQRCode(FData, Ord(FEncoding));
  FRows := Length(FElements) + FQuietZone * 2;
  FColumns := FRows;
end;

end.