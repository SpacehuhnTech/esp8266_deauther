{ ******************************************************* }
{ 配置行框架单元 }
{ ******************************************************* }
{ ******************************************************* }
{ 本软件使用MIT协议. }
{ 发布本软件的目的是希望它能够在一定程度上帮到您. }
{ 编写者: Vowstar <vowstar@gmail.com>, NODEMCU开发组. }
{ ******************************************************* }
unit UnitFrameConfigLine;

interface

uses
  Winapi.Windows, Winapi.Messages, System.SysUtils, System.Variants,
  System.Classes, StrUtils,
  Vcl.Graphics, Vcl.Controls, Vcl.Forms, Vcl.Dialogs, Vcl.Buttons, Vcl.StdCtrls,
  Vcl.ExtCtrls, Vcl.Imaging.pngimage;

type
  TFrameConfigLine = class(TFrame)
    CheckBoxEnable: TCheckBox;
    ComboBoxPath: TComboBox;
    ComboBoxOffset: TComboBox;
    GridPanelConfigLine: TGridPanel;
    ImageOpenFile: TImage;
    OpenDialogBinaries: TOpenDialog;
    TimerCheck: TTimer;
    procedure ImageOpenFileClick(Sender: TObject);
    procedure ComboBoxOffsetKeyPress(Sender: TObject; var Key: Char);
    procedure TimerCheckTimer(Sender: TObject);
  private
    function GetFilePath: String;
    procedure SetFilePath(const Value: String);
    function GetOffset: String;
    procedure SetOffset(const Value: String);
    function GetChecked: Boolean;
    procedure SetChecked(const Value: Boolean);
    { Private declarations }
  public
    { Public declarations }
    function GetMemoryStream(var MemoryStream: TMemoryStream): Boolean;
    function GetBaseAddress(var BaseAddress: UInt32): Boolean;
    function IsFilePathValid: Boolean;
    property FilePath: String read GetFilePath write SetFilePath;
    property Offset: String read GetOffset write SetOffset;
    property Checked: Boolean read GetChecked write SetChecked;
  end;

implementation

{$R *.dfm}

const
  ResType = 'BIN';
  Prefix = 'INTERNAL://';

function HexToInt(HexStr: string): Int64;
var
  RetVar: Int64;
  i: byte;
begin
  HexStr := UpperCase(HexStr);
  if HexStr[length(HexStr)] = 'H' then
    Delete(HexStr, length(HexStr), 1);
  RetVar := 0;

  for i := 1 to length(HexStr) do
  begin
    RetVar := RetVar shl 4;
    if HexStr[i] in ['0' .. '9'] then
      RetVar := RetVar + (byte(HexStr[i]) - 48)
    else if HexStr[i] in ['A' .. 'F'] then
      RetVar := RetVar + (byte(HexStr[i]) - 55)
    else
    begin
      RetVar := 0;
      break;
    end;
  end;

  Result := RetVar;
end;

function IntToBin(IValue: Int64; NumBits: word = 64): string;
var
  RetVar: string;
  i, ILen: byte;
begin
  RetVar := '';

  case NumBits of
    32:
      IValue := dword(IValue);
    16:
      IValue := word(IValue);
    8:
      IValue := byte(IValue);
  end;

  while IValue <> 0 do
  begin
    RetVar := Char(48 + (IValue and 1)) + RetVar;
    IValue := IValue shr 1;
  end;

  if RetVar = '' then
    RetVar := '0';
  Result := RetVar;
end;

function BinToInt(BinStr: string): Int64;
var
  i: byte;
  RetVar: Int64;
begin
  BinStr := UpperCase(BinStr);
  if BinStr[length(BinStr)] = 'B' then
    Delete(BinStr, length(BinStr), 1);
  RetVar := 0;
  for i := 1 to length(BinStr) do
  begin
    if not(BinStr[i] in ['0', '1']) then
    begin
      RetVar := 0;
      break;
    end;
    RetVar := (RetVar shl 1) + (byte(BinStr[i]) and 1);
  end;

  Result := RetVar;
end;

procedure TFrameConfigLine.ImageOpenFileClick(Sender: TObject);
begin
  if (OpenDialogBinaries.Execute(Handle)) then
  begin
    if (OpenDialogBinaries.FileName <> '') then
    begin
      ComboBoxPath.Text := OpenDialogBinaries.FileName;
      ComboBoxPath.Items.Add(ComboBoxPath.Text);
    end;
  end;
end;

procedure TFrameConfigLine.SetFilePath(const Value: String);
begin
  ComboBoxPath.Text := Value;
  ComboBoxPath.SelectAll;
end;

procedure TFrameConfigLine.ComboBoxOffsetKeyPress(Sender: TObject;
  var Key: Char);
begin
  if (not CharInSet(Key, [#8, #127])) then
    if (not CharInSet(Key, ['0' .. '9', 'A' .. 'F'])) then
      if (CharInSet(Key, ['a' .. 'f'])) then
      begin
        Key := UpCase(Key);
      end
      else if (CharInSet(Key, ['X', 'x'])) then
      begin
        // if (((length(ComboBoxOffset.Text) = 1) or (ComboBoxOffset.SelStart = 1))
        // And (ComboBoxOffset.Text[1] = '0')) then
        // begin
        Key := 'x';
        // end
        // else
        // begin
        // Key := #0;
        // end;
        // end
        // else
        // begin
        // Key := #0;
      end;
end;

function TFrameConfigLine.GetFilePath: String;
begin
  Result := ComboBoxPath.Text;
end;

procedure TFrameConfigLine.SetOffset(const Value: String);
begin
  ComboBoxOffset.Text := Value;
  ComboBoxOffset.SelectAll;
end;

procedure TFrameConfigLine.TimerCheckTimer(Sender: TObject);
begin
  if (FilePath <> '') then
  begin
    ComboBoxPath.StyleElements := [seFont, seBorder];
    if (IsFilePathValid) then
    begin
      ComboBoxPath.Color := clGreen;
    end
    else
    begin
      ComboBoxPath.Color := clRed;
    end;
  end
  else
  begin
    ComboBoxPath.StyleElements := [seFont, seClient, seBorder];
  end;
end;

function TFrameConfigLine.GetOffset: String;
begin
  Result := ComboBoxOffset.Text;
end;

procedure TFrameConfigLine.SetChecked(const Value: Boolean);
begin
  CheckBoxEnable.Checked := Value;
end;

function TFrameConfigLine.GetChecked: Boolean;
begin
  Result := CheckBoxEnable.Checked;
end;

function TFrameConfigLine.IsFilePathValid: Boolean;
var
  ResName: string;
  HResInfo: THandle;
begin
  Result := False;
  if (FilePath <> '') then
  begin
    if (Pos(Prefix, FilePath) <> 0) then
    begin
      ResName := ReplaceStr(FilePath, Prefix, '');
      HResInfo := FindResource(Hinstance, Pchar(ResName), Pchar(ResType));
      if (HResInfo <> 0) then
      begin
        Result := True;
      end;
    end
    else
    begin
      if (FileExists(FilePath)) then
        Result := True;
    end;
  end;
end;

function TFrameConfigLine.GetMemoryStream(var MemoryStream
  : TMemoryStream): Boolean;
var
  ResStream: TResourceStream;
  ResName: string;
begin
  Result := False;
  if ((MemoryStream <> nil) And (FilePath <> '')) then
  begin
    if (IsFilePathValid) then
      if (Pos(Prefix, FilePath) > 0) then
      begin
        ResName := ReplaceStr(FilePath, Prefix, '');
        try
          ResStream := TResourceStream.Create(Hinstance, ResName,
            Pchar(ResType));
          ResStream.Position := 0;
          MemoryStream.LoadFromStream(ResStream);
          ResStream.Free;
          Result := True;
        except
          Result := False;
        end;
      end
      else
      begin
        try
          MemoryStream.LoadFromFile(FilePath);
          Result := True;
        except
          Result := False;
        end;
      end;
  end;
end;

function TFrameConfigLine.GetBaseAddress(var BaseAddress: UInt32): Boolean;
Var
  BaseAddressHex: String;
begin
  Result := False;
  if (Offset <> '') then
  begin
    BaseAddressHex := ReplaceStr(Offset, '0x', '');
    BaseAddress := HexToInt(BaseAddressHex);
    Result := True;
  end;
end;

end.
