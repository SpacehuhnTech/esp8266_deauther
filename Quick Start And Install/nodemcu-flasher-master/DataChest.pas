{
  ======================================
  Language:
  Object Pascal
  Name:
  DataChest
  数据存储柜
  Description:
  使用动态index存储数据
  Writer:
  黄锐( vowstar@gmail.com )
  Date:
  2010-4-8 12:54:43
  36816
  ======================================
}
unit DataChest;

interface

uses
  Classes, SysUtils, xmldom, XMLDoc, XMLIntf, EncdDecd;

const
  ChestTypeEmpty = 0;
  ChestTypeInteger = 1;
  ChestTypeReal = 2;
  ChestTypeBoolean = 3;
  ChestTypeString = 4;
  ChestTypePointer = 5;

type
  {
    ======================================
    TChestElement
    不建副本到内存
    ======================================
    }
  TChestElement = class(TObject)
    ChestType: Word;
    Value: Pointer;
    Name: String;
  end;

  TChestString = class(TObject)
    Value: string;
  end;

  {
    ======================================
    TChest
    不建副本到内存
    ======================================
    }
  TChest = class(TObject)
  const
    Version = '1.0.0.0';
  private

    class var List: TList;
    class var Checked: Boolean;
    class var Sorted: Boolean;
    class var _Publisher: string;
    class var FDuplicates: TDuplicates;
    class function SelfCheck: Boolean; static;

    class procedure SortByName; static;
    class function GetValue(index: string): Pointer; static;
    class function GetValueAndType(index: string;
      var ChestType: Word): Pointer; static;
    class procedure SetValue(index: string; const Value: Pointer); static;
    class procedure SetValueAndType(index: string; const Value: Pointer;
      ChestType: Word = ChestTypeEmpty); static;
    class function GetCount: integer; static;
    class procedure _Delete(index: integer); overload; static;
    class function GetItem(index: integer): TChestElement; static;
    class procedure SetItem(index: integer; const Value: TChestElement); static;
  public
    class property Item[index: integer]
      : TChestElement read GetItem write SetItem; default;

    class property AsPointer[index: string]
      : Pointer read GetValue write SetValue default nil;
    class property Count: integer read GetCount default 0;
    class property Publisher: string read _Publisher write _Publisher;
    class property Duplicates: TDuplicates read FDuplicates write FDuplicates;
    class function Find(const S: string; var Index: integer): Boolean; static;
    class procedure Clear; static;
    class procedure _Delete(index: string); overload; static;
    class procedure DoInitialization; static;
    class procedure DoFinalization; static;

  end;

  {
    ======================================
    TMemChest
    不建副本到内存
    ======================================
    }
  TMemChest = class(TChest)
  private
    class var TempList: TList;
    class procedure Delete(index: integer); overload; static;
    class function GetXML: WideString; static;
    class procedure SetXML(const Value: WideString); static;
  public
    class procedure FreeMemory; static;
    class procedure Delete(index: string); overload; static;
    class procedure SaveToXMLFile(FileName: string); static;
    class procedure LoadFromXMLFile(FileName: string); static;
    class procedure SaveToEncryptFile(FileName: string); static;
    class procedure LoadFromEncryptFile(FileName: string); static;
    class procedure SaveToFile(FileName: string); static;
    class procedure LoadFromFile(FileName: string); static;
    class property XML: WideString read GetXML write SetXML;
  end;

  {
    ======================================
    TIntChest
    建副本到内存
    ======================================
    }
  TIntChest = class(TMemChest)
  private
    class function GetIntValue(index: string): integer; static;
    class procedure SetIntValue(index: string; const Value: integer); static;
    class function GetStringValue(index: string): String; static;
    class procedure SetStringValue(index: string; const Value: String); static;
  public
    class property AsInt[index: string]
      : integer read GetIntValue write SetIntValue; default;
    class property AsString[index: string]
      : String read GetStringValue write SetStringValue;
  end;

  {
    ======================================
    TRealChest
    建副本到内存
    ======================================
    }
  TRealChest = class(TMemChest)
  private
    class function GetRealValue(index: string): Real; static;
    class procedure SetRealValue(index: string; const Value: Real); static;
    class function GetIntValue(index: string): integer; static;
    class function GetStringValue(index: string): String; static;
    class procedure SetIntValue(index: string; const Value: integer); static;
    class procedure SetStringValue(index: string; const Value: String); static;
  public
    class property AsReal[index: string]
      : Real read GetRealValue write SetRealValue; default;
    class property AsInt[index: string]
      : integer read GetIntValue write SetIntValue;
    class property AsString[index: string]
      : String read GetStringValue write SetStringValue;
  end;

  {
    ======================================
    TBooleanChest
    建副本到内存
    ======================================
    }
  TBooleanChest = class(TMemChest)
  private
    class function GetRealValue(index: string): Real; static;
    class procedure SetRealValue(index: string; const Value: Real); static;
    class function GetIntValue(index: string): integer; static;
    class function GetStringValue(index: string): String; static;
    class procedure SetIntValue(index: string; const Value: integer); static;
    class procedure SetStringValue(index: string; const Value: String); static;
    class function GetBooleanValue(index: string): Boolean; static;
    class procedure SetBooleanValue(index: string; const Value: Boolean);
      static;
  public
    class property AsBoolean[index: string]
      : Boolean read GetBooleanValue write SetBooleanValue; default;
    class property AsReal[index: string]
      : Real read GetRealValue write SetRealValue;
    class property AsInt[index: string]
      : integer read GetIntValue write SetIntValue;
    class property AsString[index: string]
      : String read GetStringValue write SetStringValue;
  end;

  {
    ======================================
    TStringChest
    建副本到内存
    ======================================
    }
  TStringChest = class(TMemChest)
  private
    class function GetStringValue(index: string): string; static;
    class procedure SetStringValue(index: string; const Value: string); static;
  public
    class property AsString[index: string]
      : string read GetStringValue write SetStringValue; default;
  end;

  {
    ======================================
    TObjectChest
    不建副本到内存
    ======================================
    }
  TObjectChest = class(TChest)
  private
    class function GetObjectValue(index: string): TObject; static;
    class procedure SetObjectValue(index: string; const Value: TObject); static;
  public
    class property AsObject[index: string]
      : TObject read GetObjectValue write SetObjectValue; default;
  end;

  {
    ======================================
    TComponentChest
    不建副本到内存
    ======================================
    }
  TComponentChest = class(TObjectChest)
  private
    class function GetComponentValue(index: string): TComponent; static;
    class procedure SetComponentValue(index: string; const Value: TComponent);
      static;
  public
    class property AsComponent[index: string]
      : TComponent read GetComponentValue write
      SetComponentValue; default;
  end;

  {
    ======================================
    XML interface(AutoCreated)
    ======================================
    }
  { Forward Decls }

  IXMLDataChestType = interface;
  IXMLPublisherType = interface;
  IXMLDataType = interface;
  IXMLDataTypeList = interface;

  { IXMLDataChestType }

  IXMLDataChestType = interface(IXMLNode)
    ['{02B0FB9E-6CEA-44DE-BB53-63C688856EB7}']
    { Property Accessors }
    function Get_Publisher: IXMLPublisherType;
    function Get_Data: IXMLDataTypeList;
    { Methods & Properties }
    property Publisher: IXMLPublisherType read Get_Publisher;
    property Data: IXMLDataTypeList read Get_Data;
  end;

  { IXMLPublisherType }

  IXMLPublisherType = interface(IXMLNode)
    ['{B2149CEA-4B15-4BC1-B12A-D396BBADE025}']
    { Property Accessors }
    function Get_Info: UnicodeString;
    function Get_Version: UnicodeString;
    procedure Set_Info(Value: UnicodeString);
    procedure Set_Version(Value: UnicodeString);
    { Methods & Properties }
    property Info: UnicodeString read Get_Info write Set_Info;
    property Version: UnicodeString read Get_Version write Set_Version;
  end;

  { IXMLDataType }

  IXMLDataType = interface(IXMLNode)
    ['{956479AF-F38D-46E3-94AA-03EE6464F9E8}']
    { Property Accessors }
    function Get_Name: UnicodeString;
    function Get_Type_: UnicodeString;
    function Get_Value: UnicodeString;
    procedure Set_Name(Value: UnicodeString);
    procedure Set_Type_(Value: UnicodeString);
    procedure Set_Value(Value: UnicodeString);
    { Methods & Properties }
    property Name: UnicodeString read Get_Name write Set_Name;
    property Type_: UnicodeString read Get_Type_ write Set_Type_;
    property Value: UnicodeString read Get_Value write Set_Value;
  end;

  { IXMLDataTypeList }

  IXMLDataTypeList = interface(IXMLNodeCollection)
    ['{9A322421-9DD5-48C2-ACB0-4E0A9A0AF866}']
    { Methods & Properties }
    function Add: IXMLDataType;
    function Insert(const Index: integer): IXMLDataType;

    function Get_Item(Index: integer): IXMLDataType;
    property Items[Index: integer]: IXMLDataType read Get_Item; default;
  end;

  { Forward Decls }

  TXMLDataChestType = class;
  TXMLPublisherType = class;
  TXMLDataType = class;
  TXMLDataTypeList = class;

  { TXMLDataChestType }

  TXMLDataChestType = class(TXMLNode, IXMLDataChestType)
  private
    FData: IXMLDataTypeList;
  protected
    { IXMLDataChestType }
    function Get_Publisher: IXMLPublisherType;
    function Get_Data: IXMLDataTypeList;
  public
    procedure AfterConstruction; override;
  end;

  { TXMLPublisherType }

  TXMLPublisherType = class(TXMLNode, IXMLPublisherType)
  protected
    { IXMLPublisherType }
    function Get_Info: UnicodeString;
    function Get_Version: UnicodeString;
    procedure Set_Info(Value: UnicodeString);
    procedure Set_Version(Value: UnicodeString);
  end;

  { TXMLDataType }

  TXMLDataType = class(TXMLNode, IXMLDataType)
  protected
    { IXMLDataType }
    function Get_Name: UnicodeString;
    function Get_Type_: UnicodeString;
    function Get_Value: UnicodeString;
    procedure Set_Name(Value: UnicodeString);
    procedure Set_Type_(Value: UnicodeString);
    procedure Set_Value(Value: UnicodeString);
  end;

  { TXMLDataTypeList }

  TXMLDataTypeList = class(TXMLNodeCollection, IXMLDataTypeList)
  protected
    { IXMLDataTypeList }
    function Add: IXMLDataType;
    function Insert(const Index: integer): IXMLDataType;

    function Get_Item(Index: integer): IXMLDataType;
  end;

  {
    ======================================
    Global Functions
    ======================================
    }
function ChestTypeToString(ChestType: Word): string;
function StringToChestType(S: string): Word;

function GetDataChest(Doc: IXMLDocument): IXMLDataChestType;
function LoadDataChest(const FileName: string): IXMLDataChestType;
function NewDataChest: IXMLDataChestType;

const
  TargetNamespace = '';
  {
    ======================================
    ChestValue
    ======================================
    }

var
  Chest: TChest;
  IntChest: TIntChest;
  RealChest: TRealChest;
  StringChest: TStringChest;

  {
    ======================================
    ======================================
    ======================================
    ======================================
    Implementation
    ======================================
    ======================================
    ======================================
    }

implementation

{ TChest }

class function TChest.SelfCheck: Boolean;
begin
  if (not TChest.Checked) then
  begin
    if not Assigned(TChest.List) then
    begin
      TChest.List := TList.Create;
      Result := False;
    end
    else
      Result := True;
    TChest.Checked := True;
  end
  else
    Result := True;
end;

class procedure TChest.Clear;
var
  i: integer;
  Item: Pointer;
begin
  for i := 0 to TChest.List.Count - 1 do
  begin
    Item := TChest.List.Items[i];
    TChestElement(Item^).Free;
    FreeMem(Item, SizeOf(TChestElement));
  end;
  TChest.List.Clear;
end;

class procedure TChest._Delete(index: integer);
var
  Item: Pointer;
begin
  Item := TChest.List.Items[index];
  TChestElement(Item^).Free;
  FreeMem(Item, SizeOf(TChestElement));
  TChest.List.Delete(index);
  TChest.Sorted := False;
end;

class procedure TChest._Delete(index: string);
var
  i: integer;
begin
  if not TChest.Sorted then
    TChest.SortByName;
  if TChest.Find(index, i) then
    TChest._Delete(i);
end;

class procedure TChest.DoFinalization;
begin
  if Assigned(TChest.List) then
  begin
    TChest.Clear;
    TChest.List.Free;
    TChest.List := nil;
  end;
  TChest.Checked := False;
  TChest.Sorted := False;
end;

class procedure TChest.DoInitialization;
begin
  TChest.Checked := False;
  TChest.Sorted := False;
  TChest.SelfCheck;
  TChest.Clear;
end;

class function TChest.Find(const S: string; var Index: integer): Boolean;
var
  L, H, i, C: integer;
begin
  Result := False;
  L := 0;
  H := TChest.List.Count - 1;
  while L <= H do
  begin
    i := (L + H) shr 1;
    C := CompareText(TChestElement(TChest.List.Items[i]^).Name, S);
    if C < 0 then
      L := i + 1
    else
    begin
      H := i - 1;
      if C = 0 then
      begin
        Result := True;
        if Duplicates <> dupAccept then
          L := i;
      end;
    end;
  end;
  Index := L;
end;

class function TChest.GetCount: integer;
begin
  if Assigned(TChest.List) then
    Result := TChest.List.Count
  else
    Result := 0;
end;

class function TChest.GetItem(index: integer): TChestElement;
begin
  Result := TChestElement(TChest.List.Items[index]^);
end;

class function TChest.GetValue(index: string): Pointer;
var
  i: integer;
begin
  if not TChest.SelfCheck then
  begin
    Result := nil;
    Exit;
  end
  else
  begin
    if not TChest.Sorted then
      TChest.SortByName;
    if TChest.Find(index, i) then
    begin
      Result := TChestElement(TChest.List.Items[i]^).Value;
      Exit;
    end
    else
    begin
      Result := nil;
      Exit;
    end;
  end;
end;

class function TChest.GetValueAndType(index: string;
  var ChestType: Word): Pointer;
var
  i: integer;
  Item: Pointer;
begin
  if not TChest.SelfCheck then
  begin
    ChestType := ChestTypeEmpty;
    Result := nil;
    Exit;
  end
  else
  begin
    if not TChest.Sorted then
      TChest.SortByName;
    if TChest.Find(index, i) then
    begin
      Item := TChest.List.Items[i];
      ChestType := TChestElement(Item^).ChestType;
      Result := TChestElement(Item^).Value;
      Exit;
    end
    else
    begin
      Result := nil;
      Exit;
    end;
  end;
end;

class procedure TChest.SetItem(index: integer; const Value: TChestElement);
begin
  TChestElement((TChest.List.Items[index])^) := Value;
end;

class procedure TChest.SetValue(index: string; const Value: Pointer);
var
  i: integer;
  P: Pointer;
begin
  TChest.SelfCheck;
  if TChest.Find(index, i) then
  begin
    TChestElement((TChest.List.Items[i])^).Value := Value;
    Exit;
  end
  else
  begin
    P := GetMemory(SizeOf(TChestElement));
    TChestElement(P^) := TChestElement.Create;
    TChestElement(P^).Name := index;
    TChestElement(P^).Value := Value;
    TChest.List.Add(P);
    TChest.Sorted := False;
  end;
end;

class procedure TChest.SetValueAndType(index: string; const Value: Pointer;
  ChestType: Word = ChestTypeEmpty);
var
  i: integer;
  P, Item: Pointer;
begin
  TChest.SelfCheck;
  if TChest.Find(index, i) then
  begin
    Item := TChest.List.Items[i];
    TChestElement(Item^).Value := Value;
    TChestElement(Item^).ChestType := ChestType;
    Exit;
  end
  else
  begin
    P := GetMemory(SizeOf(TChestElement));
    TChestElement(P^) := TChestElement.Create;
    TChestElement(P^).Name := index;
    TChestElement(P^).Value := Value;
    TChestElement(P^).ChestType := ChestType;
    TChest.List.Add(P);
    TChest.Sorted := False;
  end;
end;

class procedure TChest.SortByName;
  procedure QuickSort(iLo, iHi: integer);
  var
    Lo, Hi: integer;
    Mid: string;
  begin
    Lo := iLo;
    Hi := iHi;
    Mid := TChestElement(TChest.List.Items[(Lo + Hi) div 2]^).Name;
    repeat
      while CompareText(TChestElement(TChest.List.Items[Lo]^).Name, Mid) < 0 do
        Inc(Lo);
      while CompareText(TChestElement(TChest.List.Items[Hi]^).Name, Mid) > 0 do
        Dec(Hi);
      if Lo <= Hi then
      begin
        TChest.List.Exchange(Lo, Hi);
        Inc(Lo);
        Dec(Hi);
      end;
    until Lo > Hi;
    if Hi > iLo then
      QuickSort(iLo, Hi);
    if Lo < iHi then
      QuickSort(Lo, iHi);
  end;

begin
  if not TChest.Sorted then
  begin
    if TChest.List.Count > 0 then
      QuickSort(0, TChest.List.Count - 1);
    TChest.Sorted := True;
  end;
end;

{ TMemChest }

class procedure TMemChest.Delete(index: integer);
  procedure SelfDelete(index, Size: integer);
  begin
    FreeMem(TChestElement((TMemChest.List.Items[index])^).Value, Size);
    TChest._Delete(index);
  end;

begin
  case TChestElement((TChest.List.Items[index])^).ChestType of
    ChestTypeInteger:
      SelfDelete(index, SizeOf(integer));
    ChestTypeReal:
      SelfDelete(index, SizeOf(Real));
    ChestTypeBoolean:
      SelfDelete(index, SizeOf(Boolean));
    ChestTypeString:
      begin
        TChestString(TChestElement((TChest.List.Items[index])^).Value^).Free;
        SelfDelete(index, SizeOf(TChestString));
      end;
    ChestTypePointer:
      SelfDelete(index, SizeOf(Pointer));
  end;
  TMemChest.TempList.Delete(index);
end;

class procedure TMemChest.Delete(index: string);
var
  i: integer;
begin
  if not TMemChest.Sorted then
    TMemChest.SortByName;
  if TMemChest.Find(index, i) then
    TMemChest.Delete(i);
end;

class procedure TMemChest.FreeMemory;
var
  i: integer;
begin
  if Assigned(TMemChest.TempList) then
  begin
    for i := TMemChest.List.Count - 1 downto 0 do
    begin
      TMemChest.Delete(i);
    end;
    TMemChest.TempList.Clear;
    TMemChest.TempList.Free;
    TMemChest.TempList := nil;
  end;
end;

class function TMemChest.GetXML: WideString;
var
  XMLDataChest: IXMLDataChestType;
  XMLData: IXMLDataType;
  XML: WideString;
  i: integer;
  Item: TChestElement;
begin
  XMLDataChest := NewDataChest;
  for i := 0 to TMemChest.Count - 1 do
  begin
    Item := TChestElement(TMemChest.List.Items[i]^);
    if (Item.Value <> nil) then
    begin
      XMLData := XMLDataChest.Data.Add;
      XMLData.Name := Item.Name;
      case Item.ChestType of
        ChestTypeInteger:
          begin
            XMLData.Type_ := 'Integer';
            XMLData.Value := IntToStr(integer(Item.Value^));
          end;
        ChestTypeBoolean:
          begin
            XMLData.Type_ := 'Boolean';
            if (Boolean(Item.Value^)) then
              XMLData.Value := 'True'
            else
              XMLData.Value := 'False';
          end;
        ChestTypeReal:
          begin
            XMLData.Type_ := 'Real';
            XMLData.Value := FloatToStr(Real(Item.Value^));
          end;
        ChestTypeString:
          begin
            XMLData.Type_ := 'String';
            XMLData.Value := TChestString(Item.Value^).Value;
          end;
      else
        begin
          XMLData.Type_ := 'Empty';
          XMLData.Value := 'Invalid';
        end;
      end;
    end;
  end;
  if TMemChest._Publisher = '' then
    XMLDataChest.Publisher.Info :=
      'DataChest,©黄锐,vowstar@gmail.com,http://hi.baidu.com/蝶晓梦'
  else
    XMLDataChest.Publisher.Info := TMemChest._Publisher;
  XMLDataChest.Publisher.Version := '1.0';
  XMLDataChest.OwnerDocument.Encoding := 'UTF-8';
  XMLDataChest.OwnerDocument.SaveToXML(XML);
  Result := XML;

end;

class procedure TMemChest.LoadFromEncryptFile(FileName: string);
var
  StringStream: TStringStream;
  Temp: WideString;
begin
  Temp := '';
  if (FileExists(FileName)) then
  begin
    StringStream := TStringStream.Create;
    StringStream.LoadFromFile(FileName);
    Temp := StringStream.ReadString(StringStream.Size);
    StringStream.Free;
  end;
  TMemChest.XML := DecodeString(Temp);
end;

class procedure TMemChest.LoadFromFile(FileName: string);
begin
  TMemChest.LoadFromEncryptFile(FileName);
end;

class procedure TMemChest.LoadFromXMLFile(FileName: string);
var
  StringList: TStringList;
begin
  StringList := TStringList.Create;
  StringList.LoadFromFile(FileName, TEncoding.UTF8);
  TMemChest.XML := StringList.Text;
  StringList.Free;
end;

class procedure TMemChest.SaveToEncryptFile(FileName: string);
var
  StringStream: TStringStream;
  Temp: string;
begin
  StringStream := TStringStream.Create;
  Temp := EncodeString(TMemChest.XML);
  StringStream.WriteString(Temp);
  StringStream.SaveToFile(FileName);
  StringStream.Free;
end;

class procedure TMemChest.SaveToFile(FileName: string);
begin
  TMemChest.SaveToEncryptFile(FileName);
end;

class procedure TMemChest.SaveToXMLFile(FileName: string);
{
  耗时操作
}
var
  StringList: TStringList;
begin
  StringList := TStringList.Create;
  StringList.Text := FormatXMLData(TMemChest.XML);
  StringList.SaveToFile(FileName, TEncoding.UTF8);
  StringList.Free;
end;

class procedure TMemChest.SetXML(const Value: WideString);
var
  XMLDataChest: IXMLDataChestType;
  XMLData: IXMLDataType;
  i: integer;
  ChestType: Word;
  Doc: IXMLDocument;
begin
  Doc := NewXMLDocument;
  Doc.LoadFromXML(Value);
  XMLDataChest := GetDataChest(Doc);
  for i := 0 to XMLDataChest.Data.Count - 1 do
  begin
    XMLData := XMLDataChest.Data.Items[i];
    ChestType := StringToChestType(XMLData.Type_);
    case ChestType of
      ChestTypeInteger:
        begin
          TIntChest.AsString[XMLData.Name] := XMLData.Value;
        end;
      ChestTypeReal:
        begin
          TRealChest.AsString[XMLData.Name] := XMLData.Value;
        end;
      ChestTypeBoolean:
        begin
          TBooleanChest.AsString[XMLData.Name] := XMLData.Value;
        end;
      ChestTypeString:
        begin
          TStringChest.AsString[XMLData.Name] := XMLData.Value;
        end;
    end;
  end;
end;

{ TIntChest }

class function TIntChest.GetIntValue(index: string): integer;
var
  P: Pointer;
  ChestType: Word;
begin
  P := TIntChest.GetValueAndType(index, ChestType);
  if (P <> nil) then
  begin
    if (ChestType = ChestTypeInteger) then
      Result := integer(P^)
    else
      Result := 0;
  end
  else
  begin
    Result := 0;
  end;
end;

class function TIntChest.GetStringValue(index: string): String;
begin
  Result := IntToStr(TIntChest.GetIntValue(index));
end;

class procedure TIntChest.SetIntValue(index: string; const Value: integer);
var
  P: ^integer;
begin
  if not Assigned(TIntChest.TempList) then
    TIntChest.TempList := TList.Create;

  GetMem(P, SizeOf(integer));
  TIntChest.TempList.Add(P);
  P^ := Value;
  TIntChest.SetValueAndType(index, P, ChestTypeInteger);
end;

class procedure TIntChest.SetStringValue(index: string; const Value: String);
begin
  TIntChest.SetIntValue(index, StrToInt(Value));
end;

{ TRealChest }

class function TRealChest.GetIntValue(index: string): integer;
begin
  Result := Round(TRealChest.GetRealValue(index));
end;

class function TRealChest.GetRealValue(index: string): Real;
var
  P: Pointer;
  ChestType: Word;
begin
  P := TRealChest.GetValueAndType(index, ChestType);
  if (P <> nil) then
  begin
    if (ChestType = ChestTypeReal) then
      Result := Real(P^)
    else
      Result := 0.0;
  end
  else
  begin
    Result := 0.0;
  end;
end;

class function TRealChest.GetStringValue(index: string): String;
begin
  Result := FloatToStr(TRealChest.GetRealValue(index));
end;

class procedure TRealChest.SetIntValue(index: string; const Value: integer);
begin
  TRealChest.SetRealValue(index, Value);
end;

class procedure TRealChest.SetRealValue(index: string; const Value: Real);
var
  P: ^Real;
begin
  if not Assigned(TRealChest.TempList) then
    TRealChest.TempList := TList.Create;

  GetMem(P, SizeOf(Real));
  TRealChest.TempList.Add(P);
  P^ := Value;
  TRealChest.SetValueAndType(index, P, ChestTypeReal);
end;

class procedure TRealChest.SetStringValue(index: string; const Value: String);
begin
  TRealChest.SetRealValue(index, StrToFloat(Value));
end;

{ TBooleanChest }

class function TBooleanChest.GetBooleanValue(index: string): Boolean;
var
  P: Pointer;
  ChestType: Word;
begin
  P := TBooleanChest.GetValueAndType(index, ChestType);
  if (P <> nil) then
  begin
    if (ChestType = ChestTypeBoolean) then
      Result := Boolean(P^)
    else
      Result := False;
  end
  else
  begin
    Result := False;
  end;
end;

class function TBooleanChest.GetIntValue(index: string): integer;
begin
  if (TBooleanChest.GetBooleanValue(index)) then
    Result := 1
  else
    Result := 0;
end;

class function TBooleanChest.GetRealValue(index: string): Real;
begin
  if (TBooleanChest.GetBooleanValue(index)) then
    Result := 1
  else
    Result := 0;
end;

class function TBooleanChest.GetStringValue(index: string): String;
begin
  if (TBooleanChest.GetBooleanValue(index)) then
    Result := 'True'
  else
    Result := 'False';
end;

class procedure TBooleanChest.SetBooleanValue(index: string;
  const Value: Boolean);
var
  P: ^Boolean;
begin
  if not Assigned(TBooleanChest.TempList) then
    TBooleanChest.TempList := TList.Create;

  GetMem(P, SizeOf(Boolean));
  TBooleanChest.TempList.Add(P);
  P^ := Value;
  TBooleanChest.SetValueAndType(index, P, ChestTypeBoolean);
end;

class procedure TBooleanChest.SetIntValue(index: string; const Value: integer);
begin
  if Value > 0 then
    TBooleanChest.SetBooleanValue(index, True)
  else
    TBooleanChest.SetBooleanValue(index, False);

end;

class procedure TBooleanChest.SetRealValue(index: string; const Value: Real);
begin
  if Value > 0.0000001 then
    TBooleanChest.SetBooleanValue(index, True)
  else
    TBooleanChest.SetBooleanValue(index, False);
end;

class procedure TBooleanChest.SetStringValue(index: string;
  const Value: String);
begin
  if UpperCase(Value) = 'TRUE' then
    TBooleanChest.SetBooleanValue(index, True)
  else if UpperCase(Value) = 'FALSE' then
    TBooleanChest.SetBooleanValue(index, False);
end;

{ TStringChest }

class function TStringChest.GetStringValue(index: string): string;
var
  P: Pointer;
  ChestType: Word;
begin
  P := TStringChest.GetValueAndType(index, ChestType);
  if (P <> nil) then
  begin
    if (ChestType = ChestTypeString) then
      Result := TChestString(P^).Value
    else
      Result := '';
  end
  else
  begin
    Result := '';
  end;
end;

class procedure TStringChest.SetStringValue(index: string; const Value: string);
var
  P: Pointer;

begin
  if not Assigned(TStringChest.TempList) then
    TStringChest.TempList := TList.Create;
  GetMem(P, SizeOf(TChestString));
  TChestString(P^) := TChestString.Create;
  TChestString(P^).Value := Value;
  TStringChest.TempList.Add(P);
  TStringChest.SetValueAndType(index, P, ChestTypeString);
end;

{ TObjectChest }

class function TObjectChest.GetObjectValue(index: string): TObject;
var
  P: Pointer;
begin
  P := TObjectChest.GetValue(index);
  if (P <> nil) then
  begin
    Result := TObject(P^);
  end
  else
  begin
    Result := nil;
  end;
end;

class procedure TObjectChest.SetObjectValue(index: string;
  const Value: TObject);
var
  P: ^TObject;
begin
  P := @Value;
  TObjectChest.SetValue(index, P);
end;

{ TComponentChest }

class function TComponentChest.GetComponentValue(index: string): TComponent;
begin
  Result := TComponent(TComponentChest.GetObjectValue(index));
end;

class procedure TComponentChest.SetComponentValue(index: string;
  const Value: TComponent);
begin
  TComponentChest.SetObjectValue(index, TObject(Value));
end;

{ TXMLDataChestType }

procedure TXMLDataChestType.AfterConstruction;
begin
  RegisterChildNode('Publisher', TXMLPublisherType);
  RegisterChildNode('Data', TXMLDataType);
  FData := CreateCollection(TXMLDataTypeList, IXMLDataType, 'Data')
    as IXMLDataTypeList;
  inherited;
end;

function TXMLDataChestType.Get_Publisher: IXMLPublisherType;
begin
  Result := ChildNodes['Publisher'] as IXMLPublisherType;
end;

function TXMLDataChestType.Get_Data: IXMLDataTypeList;
begin
  Result := FData;
end;

{ TXMLPublisherType }

function TXMLPublisherType.Get_Info: UnicodeString;
begin
  Result := ChildNodes['Info'].Text;
end;

procedure TXMLPublisherType.Set_Info(Value: UnicodeString);
begin
  ChildNodes['Info'].NodeValue := Value;
end;

function TXMLPublisherType.Get_Version: UnicodeString;
begin
  Result := ChildNodes['Version'].Text;
end;

procedure TXMLPublisherType.Set_Version(Value: UnicodeString);
begin
  ChildNodes['Version'].NodeValue := Value;
end;

{ TXMLDataType }

function TXMLDataType.Get_Name: UnicodeString;
begin
  Result := AttributeNodes['name'].Text;
end;

procedure TXMLDataType.Set_Name(Value: UnicodeString);
begin
  SetAttribute('name', Value);
end;

function TXMLDataType.Get_Type_: UnicodeString;
begin
  Result := ChildNodes['Type'].Text;
end;

procedure TXMLDataType.Set_Type_(Value: UnicodeString);
begin
  ChildNodes['Type'].NodeValue := Value;
end;

function TXMLDataType.Get_Value: UnicodeString;
begin
  Result := ChildNodes['Value'].Text;
end;

procedure TXMLDataType.Set_Value(Value: UnicodeString);
begin
  ChildNodes['Value'].NodeValue := Value;
end;

{ TXMLDataTypeList }

function TXMLDataTypeList.Add: IXMLDataType;
begin
  Result := AddItem(-1) as IXMLDataType;
end;

function TXMLDataTypeList.Insert(const Index: integer): IXMLDataType;
begin
  Result := AddItem(Index) as IXMLDataType;
end;

function TXMLDataTypeList.Get_Item(Index: integer): IXMLDataType;
begin
  Result := List[Index] as IXMLDataType;
end;

{ Global Functions }
function ChestTypeToString(ChestType: Word): string;
begin
  case ChestType of
    ChestTypeInteger:
      begin
        Result := 'Integer';
      end;
    ChestTypeBoolean:
      begin
        Result := 'Boolean';
      end;
    ChestTypeReal:
      begin
        Result := 'Real';
      end;
    ChestTypeString:
      begin
        Result := 'String';
      end;
  else
    begin
      Result := 'Empty';
    end;
  end;
end;

function StringToChestType(S: string): Word;
var
  Temp: string;
  ChestType: Word;
begin
  Temp := UpperCase(S);
  if Temp = 'INTEGER' then
    ChestType := ChestTypeInteger
  else if Temp = 'REAL' then
    ChestType := ChestTypeReal
  else if Temp = 'BOOLEAN' then
    ChestType := ChestTypeBoolean
  else if Temp = 'STRING' then
    ChestType := ChestTypeString
  else
    ChestType := ChestTypeEmpty;
  Result := ChestType;
end;

function GetDataChest(Doc: IXMLDocument): IXMLDataChestType;
begin
  Result := Doc.GetDocBinding('DataChest', TXMLDataChestType, TargetNamespace)
    as IXMLDataChestType;
end;

function LoadDataChest(const FileName: string): IXMLDataChestType;
begin
  Result := LoadXMLDocument(FileName).GetDocBinding('DataChest',
    TXMLDataChestType, TargetNamespace) as IXMLDataChestType;
end;

function NewDataChest: IXMLDataChestType;
begin
  Result := NewXMLDocument.GetDocBinding('DataChest', TXMLDataChestType,
    TargetNamespace) as IXMLDataChestType;
end;

initialization

TChest.DoInitialization;

{
  ======================================
  ======================================
  ======================================
  ======================================
  ======================================
}
finalization

TIntChest.FreeMemory;
TRealChest.FreeMemory;
TStringChest.FreeMemory;
TChest.DoFinalization;

end.
