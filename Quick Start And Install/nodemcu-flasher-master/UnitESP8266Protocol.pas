{*******************************************************}
{                   ESP8266烧写协议单元                 }
{*******************************************************}
{*******************************************************}
{ 本软件使用MIT协议.                                    }
{ 发布本软件的目的是希望它能够在一定程度上帮到您.       }
{ 编写者: Vowstar <vowstar@gmail.com>, NODEMCU开发组.   }
{*******************************************************}
unit UnitESP8266Protocol;

interface

const
  ESP_PROTOCOL_IDENTIFIER = RawByteString(#$C0);
  ESP_PROTOCOL_ACK = RawByteString(#$00#$C0);
  ESP_HANDSHAKE = RawByteString
    (#$C0#$00#$08#$24#$00#$78#$01#$3A#$00#$07#$07#$12#$20#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$55#$C0);
  ESP_HANDSHAKE_ACK = RawByteString
    (#$C0#$01#$08#$02#$00#$07#$07#$12#$20#$00#$00#$C0);
  ESP_SET_BASE_ADDRESS = RawByteString(#$C0#$00#$02#$10#$00#$78#$01#$3A#$00) +
  {File length (aligned with 4 bit)} RawByteString(#$00#$00#$00#$00) +
  {Total sectors} RawByteString(#$00#$00) +
    RawByteString(#$00#$00#$00#$04#$00#$00) +
  {Based address} RawByteString(#$00#$00#$00#$00) + RawByteString(#$C0);
  ESP_SET_BASE_ADDRESS_ACK = RawByteString
    (#$C0#$01#$02#$02#$00#$07#$07#$12#$20#$00#$00#$C0);
  ESP_SEND_DATA = RawByteString(#$C0#$00#$03) +
  {Data length + 16 (aligned with 4 bit)} RawByteString(#$10#$04) +
  {Data Xor Check} RawByteString(#$EF) + RawByteString(#$00#$00#$00) +
  {Data length (aligned with 4 bit)} RawByteString(#$00#$04) +
    RawByteString(#$00#$00) +
  {Sector index} RawByteString(#$01#$00#$00#$00) +
    RawByteString(#$00#$00#$00#$00#$00#$00#$00#$00);
  ESP_SEND_DATA_ACK = RawByteString
    (#$C0#$01#$03#$02#$00#$07#$07#$12#$20#$00#$00#$C0);
  ESP_RUN = RawByteString
    (#$C0#$00#$04#$04#$00#$07#$07#$12#$20#$00#$00#$00#$00#$C0);
  ESP_RUN_ACK = RawByteString(#$C0#$01#$04#$02#$00#$07#$07#$12#$20#$00#$00#$C0);

  ESP_READ_REG = RawByteString(#$C0#$00#$0A#$04#$00) +
  {REG Value} RawByteString(#$07#$07#$12#$20) +
  {REG Address} RawByteString(#$50#$00#$F0#$3F) + RawByteString(#$C0);
  ESP_READ_REG_ACK = RawByteString(#$C0#$01#$0A#$02#$00) +
  {REG Value} RawByteString(#$00#$00#$8A#$FB) + RawByteString(#$00#$00#$C0);

type
  TEspSetBaseAddress = packed record
    Header: array [1 .. 9] of Byte;
    DataLen: UInt32;
    Count: UInt16;
    Body: array [1 .. 6] of Byte;
    BaseAddress: UInt32;
    Footer: array [1 .. 1] of Byte;
  end;

  PEspSetBaseAddress = ^TEspSetBaseAddress;

  TEspSendData = packed record
    Header: array [1 .. 3] of Byte;
    PacketLen: UInt16;
    XorCheck: UInt8;
    Body1: array [1 .. 3] of Byte;
    DataLen: UInt16;
    Body2: array [1 .. 2] of Byte;
    SectorIndex: UInt32;
    Footer: array [1 .. 8] of Byte;
  end;

  PEspSendData = ^TEspSendData;

  TESPReadReg = packed record
    Header: array [1 .. 5] of Byte;
    RegValue: UInt32;
    RegAddr: UInt32;
    Footer: array [1 .. 1] of Byte;
  end;

  PESPReadReg = ^TESPReadReg;

  TESPReadRegAck = packed record
    Header: array [1 .. 5] of Byte;
    RegValue: UInt32;
    Footer: array [1 .. 3] of Byte;
  end;

  PESPReadRegAck = ^TESPReadRegAck;

implementation

end.
