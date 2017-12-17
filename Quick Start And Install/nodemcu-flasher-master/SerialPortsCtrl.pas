// -----------------------------------------------------------------------------
// 本代码来源于互联网,感谢原作者.
// http://bbs.csdn.net/topics/310211601
// -----------------------------------------------------------------------------
// 名称: SerialPortsCtrl
// 备注: 串行接口枚举
// -----------------------------------------------------------------------------
unit SerialPortsCtrl;

interface

uses
  Windows, Classes, ExtCtrls;
procedure EnumComPorts(Ports: TStrings);

implementation

procedure EnumComPorts(Ports: TStrings);
var
  KeyHandle: HKEY;
  ErrCode, Index: Integer;
  ValueName, Data: string;
  ValueLen, DataLen, ValueType: DWORD;
  TmpPorts: TStringList;
begin
  ErrCode := RegOpenKeyEx(HKEY_LOCAL_MACHINE, 'HARDWARE\DEVICEMAP\SERIALCOMM',
    0, KEY_READ, KeyHandle);

  if ErrCode <> ERROR_SUCCESS then
    Exit; // raise EComPort.Create(CError_RegError, ErrCode);

  TmpPorts := TStringList.Create;
  try
    Index := 0;
    repeat
      ValueLen := 256;
      DataLen := 256;
      SetLength(ValueName, ValueLen);
      SetLength(Data, DataLen);
      ErrCode := RegEnumValue(KeyHandle, Index, PChar(ValueName),
        Cardinal(ValueLen), nil, @ValueType, PByte(PChar(Data)), @DataLen);

      if ErrCode = ERROR_SUCCESS then
      begin
        SetLength(Data, DataLen);
        TmpPorts.Add(Data);
        Inc(Index);
      end
      else if ErrCode <> ERROR_NO_MORE_ITEMS then
        Exit; // raise EComPort.Create(CError_RegError, ErrCode);

    until (ErrCode <> ERROR_SUCCESS);

    TmpPorts.Sort;
    Ports.Assign(TmpPorts);
  finally
    RegCloseKey(KeyHandle);
    TmpPorts.Free;
  end;

end;

end.
