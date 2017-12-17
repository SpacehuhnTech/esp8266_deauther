// Communications
//
// David Wann.COMM32.PAS Version 1.0
// This Communications Component is implemented using separate Read and Write
// threads. Messages from the threads are posted to the Comm control which is
// an invisible window. To handle data from the comm port, simply
// attach a handler to 'OnReceiveData'. There is no need to free the memory
// buffer passed to this handler. If TAPI is used to open the comm port, some
// changes to this component are needed ('StartComm' currently opens the comm
// port). The 'OnRequestHangup' event is included to assist this.
//
// David Wann
// Stamina Software
// 28/02/96
// davidwann@hunterlink.net.au
//
//
// This component is totally free(copyleft), you can do anything in any
// purpose EXCEPT SELL IT ALONE.
//
//
// Author: Small-Pig Team         in Taiwan R.O.C.
// Email   : spigteam@vlsi.ice.cycu.edu.tw
// Date : 1997/5/9
//
// Version 1.01     1996/9/4
// - Add setting Parity, Databits, StopBits
// - Add setting Flowcontrol:Dtr-Dsr, Cts-Rts, Xon-Xoff
// - Add setting Timeout information for read/write
//
// Version 1.02     1996/12/24
// - Add Sender parameter to TReceiveDataEvent
//
// Version 2.0      1997/4/15
// - Support separatly DTR/DSR and RTS/CTS hardware flow control setting
// - Support separatly OutX and InX software flow control setting
// - Log file(for debug) may used by many comms at the same time
// - Add DSR sensitivity property
// - You can set error char. replacement when parity error
// - Let XonLim/XoffLim and XonChar/XoffChar setting by yourself
// - You may change flow-control when comm is still opened
// - Change TComm32 to TComm
// - Add OnReceiveError event handler
// - Add OnReceiveError event handler when overrun, framing error,
// parity error
// - Fix some bug
//
// Version 2.01     1997/4/19
// - Support some property for modem
// - Add OnModemStateChange event hander when RLSD(CD) change state
//
// Version 2.02     1997/4/28
// - Bug fix: When receive XOFF character, the system FAULT!!!!
//
// Version 2.5      1997/5/9
// - Add OnSendDataEmpty event handler when all data in buffer
// are sent(send-buffer become empty) this handler is called.
// You may call send data here.
// - Change the ModemState parameters in OnModemStateChange
// to ModemEvent to indicate what modem event make this call
// - Add RING signal detect. When RLSD changed state or
// RING signal was detected, OnModemStateChange handler is called
// - Change XonLim and XoffLim from 100 to 500
// - Remove TWriteThread.WriteData member
// - PostHangupCall is re-design for debuging function
// - Add a boolean property SendDataEmpty, True when send buffer
// is empty
//
// Version 2.58     2004/10/8

unit SPComm;

interface

uses
  Windows, Messages, SysUtils, Classes, Graphics, Controls, Forms, Dialogs;

const
  // messages from read/write threads
  PWM_GOTCOMMDATA = WM_USER + 1;
  PWM_RECEIVEERROR = WM_USER + 2;
  PWM_REQUESTHANGUP = WM_USER + 3;
  PWM_MODEMSTATECHANGE = WM_USER + 4;
  PWM_SENDDATAEMPTY = WM_USER + 5;

type
  TParity = (None, Odd, Even, Mark, Space);
  TStopBits = (_1, _1_5, _2);
  TByteSize = (_5, _6, _7, _8);
  TDtrControl = (DtrEnable, DtrDisable, DtrHandshake);
  TRtsControl = (RtsEnable, RtsDisable, RtsHandshake, RtsTransmissionAvailable);

  ECommsError = class(Exception);
  TReceiveDataEvent = procedure(Sender: TObject; Buffer: PAnsiChar;
    BufferLength: Word) of object;
  TReceiveErrorEvent = procedure(Sender: TObject; EventMask: DWORD) of object;
  TModemStateChangeEvent = procedure(Sender: TObject; ModemEvent: DWORD)
    of object;
  TSendDataEmptyEvent = procedure(Sender: TObject) of object;

const
  //
  // Modem Event Constant
  //
  ME_CTS = 1;
  ME_DSR = 2;
  ME_RING = 4;
  ME_RLSD = 8;

type
  TReadThread = class(TThread)
  protected
    procedure Execute; override;
  public
    hCommFile: THandle;
    hCloseEvent: THandle;
    hComm32Window: THandle;
    // --------------------------------------------------------------------------------
    FInBufferSize: Word; // 输入缓冲区大小
    FInputLen: Word; // 每次从缓冲区取字节数
    // --------------------------------------------------------------------------------

    function SetupCommEvent(lpOverlappedCommEvent: POverlapped;
      var lpfdwEvtMask: DWORD): Boolean;
    function SetupReadEvent(lpOverlappedRead: POverlapped;
      lpszInputBuffer: LPSTR; dwSizeofBuffer: DWORD;
      var lpnNumberOfBytesRead: DWORD): Boolean;
    function HandleCommEvent(lpOverlappedCommEvent: POverlapped;
      var lpfdwEvtMask: DWORD; fRetrieveEvent: Boolean): Boolean;
    function HandleReadEvent(lpOverlappedRead: POverlapped;
      lpszInputBuffer: LPSTR; dwSizeofBuffer: DWORD;
      var lpnNumberOfBytesRead: DWORD): Boolean;
    function HandleReadData(lpszInputBuffer: LPCSTR;
      dwSizeofBuffer: DWORD): Boolean;
    function ReceiveData(lpNewString: LPSTR; dwSizeofNewString: DWORD): BOOL;
    function ReceiveError(EvtMask: DWORD): BOOL;
    function ModemStateChange(ModemEvent: DWORD): BOOL;
    procedure PostHangupCall;
  end;

  TWriteThread = class(TThread)
  protected
    procedure Execute; override;
    function HandleWriteData(lpOverlappedWrite: POverlapped;
      pDataToWrite: PAnsiChar; dwNumberOfBytesToWrite: DWORD): Boolean;
  public
    hCommFile: THandle;
    hCloseEvent: THandle;
    hComm32Window: THandle;
    pFSendDataEmpty: ^Boolean;
    procedure PostHangupCall;
  end;

  TComm = class(TComponent)
  private
    { Private declarations }
    ReadThread: TReadThread;
    WriteThread: TWriteThread;
    hCommFile: THandle;
    hCloseEvent: THandle;
    FHWnd: THandle;
    FSendDataEmpty: Boolean; // True if send buffer become empty
    // --------------------------------------------------------------------------------
    FPortOpen: Boolean; // 追加端口控制字段
    FCommPort: BYTE; // 追加端口号字段
    FPortOpenError: String; // 追加端口打开错误字段
    FOutput: AnsiString; // 追加端口输出字符字段
    FInBufferSize: Word; // 输入缓冲区大小
    FInputLen: Word; // 每次从缓冲区取字节数
    // --------------------------------------------------------------------------------
    FCommName: String; // 端口名字段,MSComm为端口号CommPort
    FBaudRate: DWORD; // 波特率字段,MSComm在Settings内
    FParityCheck: Boolean; // 校验位字段,MSComm在Settings内
    FOutx_CtsFlow: Boolean;
    FOutx_DsrFlow: Boolean;
    FDtrControl: TDtrControl;
    FDsrSensitivity: Boolean;
    FTxContinueOnXoff: Boolean;
    FOutx_XonXoffFlow: Boolean;
    FInx_XonXoffFlow: Boolean;
    FReplaceWhenParityError: Boolean;
    FIgnoreNullChar: Boolean;
    FRtsControl: TRtsControl;
    FXonLimit: Word;
    FXoffLimit: Word;
    FByteSize: TByteSize;
    FParity: TParity;
    FStopBits: TStopBits;
    FXonChar: AnsiChar;
    FXoffChar: AnsiChar;
    FReplacedChar: AnsiChar;

    FReadIntervalTimeout: DWORD;
    FReadTotalTimeoutMultiplier: DWORD;
    FReadTotalTimeoutConstant: DWORD;
    FWriteTotalTimeoutMultiplier: DWORD;
    FWriteTotalTimeoutConstant: DWORD;
    FOnReceiveData: TReceiveDataEvent;
    FOnRequestHangup: TNotifyEvent;
    FOnReceiveError: TReceiveErrorEvent;
    FOnModemStateChange: TModemStateChangeEvent;
    FOnSendDataEmpty: TSendDataEmptyEvent;
    // --------------------------------------------------------------------------------
    procedure SetPortOpen(b: Boolean); // 打开端口
    function GetPortOpen: Boolean; // 打开端口
    procedure SetCommPort(CommPort: BYTE); // 设置端口号
    procedure SetOutput(Buffer: AnsiString);
    procedure SetInputLen(StrLen: Word);
    procedure SetInBufferSize(StrSize: Word);
    // --------------------------------------------------------------------------------
    procedure SetBaudRate(Rate: DWORD);
    procedure SetParityCheck(b: Boolean);
    procedure SetOutx_CtsFlow(b: Boolean);
    procedure SetOutx_DsrFlow(b: Boolean);
    procedure SetDtrControl(c: TDtrControl);
    procedure SetDsrSensitivity(b: Boolean);
    procedure SetTxContinueOnXoff(b: Boolean);
    procedure SetOutx_XonXoffFlow(b: Boolean);
    procedure SetInx_XonXoffFlow(b: Boolean);
    procedure SetReplaceWhenParityError(b: Boolean);
    procedure SetIgnoreNullChar(b: Boolean);
    procedure SetRtsControl(c: TRtsControl);
    procedure SetXonLimit(Limit: Word);
    procedure SetXoffLimit(Limit: Word);
    procedure SetByteSize(Size: TByteSize);
    procedure SetParity(p: TParity);
    procedure SetStopBits(Bits: TStopBits);
    procedure SetXonChar(c: AnsiChar);
    procedure SetXoffChar(c: AnsiChar);
    procedure SetReplacedChar(c: AnsiChar);

    procedure SetReadIntervalTimeout(v: DWORD);
    procedure SetReadTotalTimeoutMultiplier(v: DWORD);
    procedure SetReadTotalTimeoutConstant(v: DWORD);
    procedure SetWriteTotalTimeoutMultiplier(v: DWORD);
    procedure SetWriteTotalTimeoutConstant(v: DWORD);

    procedure CommWndProc(var msg: TMessage);
    procedure _SetCommState;
    procedure _SetCommTimeout;
  protected
    { Protected declarations }
    procedure CloseReadThread;
    procedure CloseWriteThread;
    procedure ReceiveData(Buffer: PAnsiChar; BufferLength: Word);
    procedure ReceiveError(EvtMask: DWORD);
    procedure ModemStateChange(ModemEvent: DWORD);
    procedure RequestHangup;
    procedure _SendDataEmpty;
  public
    { Public declarations }
    property Handle: THandle read hCommFile;
    property SendDataEmpty: Boolean read FSendDataEmpty;
    constructor Create(AOwner: TComponent); override;
    destructor Destroy; override;
    procedure StartComm;
    procedure StopComm;
    function WriteCommData(pDataToWrite: PAnsiChar;
      dwSizeofDataToWrite: Word): Boolean;
    function GetModemState: DWORD;
    procedure SetDtrRtsControl(DtrControl: TDtrControl;
      RtsControl: TRtsControl);
  published
    { Published declarations }
    // --------------------------------------------------------------------------------
    property PortOpen: Boolean read GetPortOpen write SetPortOpen default False;
    // 追加端口号属性
    property CommPort: BYTE read FCommPort write SetCommPort default 1;
    // 追加端口号属性
    property PortOpenError: String read FPortOpenError write FPortOpenError;
    // 追加只读端口打开错误字段
    property Output: AnsiString read FOutput write SetOutput; // 追加发送数据属性
    property InputLen: Word read FInputLen write SetInputLen default 1;
    property InBufferSize: Word read FInBufferSize write SetInBufferSize
      default 2048;
    // --------------------------------------------------------------------------------
    property CommName: String read FCommName write FCommName;
    property BaudRate: DWORD read FBaudRate write SetBaudRate default 9600;
    property ParityCheck: Boolean read FParityCheck write SetParityCheck
      default False;
    property Outx_CtsFlow: Boolean read FOutx_CtsFlow write SetOutx_CtsFlow
      default False;
    property Outx_DsrFlow: Boolean read FOutx_DsrFlow write SetOutx_DsrFlow
      default False;
    property DtrControl: TDtrControl read FDtrControl write SetDtrControl
      default DtrDisable;
    property DsrSensitivity: Boolean read FDsrSensitivity
      write SetDsrSensitivity default False;
    property TxContinueOnXoff: Boolean read FTxContinueOnXoff
      write SetTxContinueOnXoff default False;
    property Outx_XonXoffFlow: Boolean read FOutx_XonXoffFlow
      write SetOutx_XonXoffFlow default False;
    property Inx_XonXoffFlow: Boolean read FInx_XonXoffFlow
      write SetInx_XonXoffFlow default False;
    property ReplaceWhenParityError: Boolean read FReplaceWhenParityError
      write SetReplaceWhenParityError default False;
    property IgnoreNullChar: Boolean read FIgnoreNullChar
      write SetIgnoreNullChar default False;
    property RtsControl: TRtsControl read FRtsControl write SetRtsControl
      default RtsDisable;
    property XonLimit: Word read FXonLimit write SetXonLimit;
    property XoffLimit: Word read FXoffLimit write SetXoffLimit;
    property ByteSize: TByteSize read FByteSize write SetByteSize;
    // property Parity: TParity read FParity write FParity;
    property Parity: TParity read FParity write SetParity;
    property StopBits: TStopBits read FStopBits write SetStopBits;
    property XonChar: AnsiChar read FXonChar write SetXonChar;
    property XoffChar: AnsiChar read FXoffChar write SetXoffChar;
    property ReplacedChar: AnsiChar read FReplacedChar write SetReplacedChar;

    property ReadIntervalTimeout: DWORD read FReadIntervalTimeout
      write SetReadIntervalTimeout;
    property ReadTotalTimeoutMultiplier: DWORD read FReadTotalTimeoutMultiplier
      write SetReadTotalTimeoutMultiplier;
    property ReadTotalTimeoutConstant: DWORD read FReadTotalTimeoutConstant
      write SetReadTotalTimeoutConstant;
    property WriteTotalTimeoutMultiplier: DWORD
      read FWriteTotalTimeoutMultiplier write SetWriteTotalTimeoutMultiplier;
    property WriteTotalTimeoutConstant: DWORD read FWriteTotalTimeoutConstant
      write SetWriteTotalTimeoutConstant;

    property OnReceiveData: TReceiveDataEvent read FOnReceiveData
      write FOnReceiveData;
    property OnReceiveError: TReceiveErrorEvent read FOnReceiveError
      write FOnReceiveError;
    property OnModemStateChange: TModemStateChangeEvent read FOnModemStateChange
      write FOnModemStateChange;
    property OnRequestHangup: TNotifyEvent read FOnRequestHangup
      write FOnRequestHangup;
    property OnSendDataEmpty: TSendDataEmptyEvent read FOnSendDataEmpty
      write FOnSendDataEmpty;
  end;

const
  // This is the message posted to the WriteThread
  // When we have something to write.
  PWM_COMMWRITE = WM_USER + 1;

  // Default size of the Input Buffer used by this code.
  // --------------------------------------------------------------------------------
  // 废除，用InBufferSize属性替代，增强灵活性
  // INPUTBUFFERSIZE = 2048;
  // INPUTBUFFERSIZE = 1;
  // --------------------------------------------------------------------------------
  RFCOMM = 340;
procedure Register;

implementation

(* **************************************************************************** *)
// TComm PUBLIC METHODS
(* **************************************************************************** *)

constructor TComm.Create(AOwner: TComponent);
begin
  inherited Create(AOwner);

  ReadThread := nil;
  WriteThread := nil;
  hCommFile := 0;
  hCloseEvent := 0;
  FSendDataEmpty := True;

  FCommPort := 1; //
  FCommName := 'COM1';
  FBaudRate := 9600;
  FParityCheck := False;
  FOutx_CtsFlow := False;
  FOutx_DsrFlow := False;
  FDtrControl := DtrDisable;
  FDsrSensitivity := False;
  FTxContinueOnXoff := False;
  FOutx_XonXoffFlow := False;
  FInx_XonXoffFlow := False;
  FReplaceWhenParityError := False;
  FIgnoreNullChar := False;
  FRtsControl := RtsDisable;
  FXonLimit := 500;
  FXoffLimit := 500;
  FByteSize := _8;
  FParity := None;
  FStopBits := _1;
  FXonChar := chr($11); // Ctrl-Q
  FXoffChar := chr($13); // Ctrl-S
  FReplacedChar := chr(0);
  FReadIntervalTimeout := 100;
  FReadTotalTimeoutMultiplier := 0;
  FReadTotalTimeoutConstant := 0;
  FWriteTotalTimeoutMultiplier := 0;
  FWriteTotalTimeoutConstant := 0;
  // -----------------------------------------------------------
  FInputLen := 1;
  FInBufferSize := 2048;
  // -----------------------------------------------------------

  if not(csDesigning in ComponentState) then
    FHWnd := AllocateHWnd(CommWndProc);
end;

destructor TComm.Destroy;
begin
  if not(csDesigning in ComponentState) then
    DeallocateHWnd(FHWnd);

  inherited Destroy;
end;

//
// FUNCTION: StartComm
//
// PURPOSE: Starts communications over the comm port.
//
// PARAMETERS:
// hNewCommFile - This is the COMM File handle to communicate with.
// This handle is obtained from TAPI.
//
// Output:
// Successful: Startup the communications.
// Failure: Raise a exception
//
// COMMENTS:
//
// StartComm makes sure there isn't communication in progress already,
// creates a Comm file, and creates the read and write threads.  It
// also configures the hNewCommFile for the appropriate COMM settings.
//
// If StartComm fails for any reason, it's up to the calling application
// to close the Comm file handle.
//
//
procedure TComm.StartComm;
var
  hNewCommFile: THandle;
begin
  FPortOpenError := '';
  FPortOpen := False;
  // Are we already doing comm?
  if (hCommFile <> 0) then
  begin
    FPortOpenError := 'This serial port already opened';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;

  hNewCommFile := CreateFile(PChar('//./' + FCommName), GENERIC_READ or
    GENERIC_WRITE, 0,
    { not shared }
    nil, { no security ?? }
    OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL or FILE_FLAG_OVERLAPPED,
    0 { template } );

  if hNewCommFile = INVALID_HANDLE_VALUE then
  begin
    FPortOpenError := 'Error opening serial port';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;

  // Is this a valid comm handle?
  // 蓝牙串口可能被识别为Unknown，为了兼容蓝牙串口增加了 FILE_TYPE_UNKNOWN
  if ((GetFileType(hNewCommFile) <> FILE_TYPE_CHAR) and
    (GetFileType(hNewCommFile) <> FILE_TYPE_UNKNOWN)) then
  begin
    CloseHandle(hNewCommFile);
    FPortOpenError := 'File handle is not a comm handle ';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;

  if not SetupComm(hNewCommFile, 4096, 4096) then
  begin
    CloseHandle(hCommFile);
    FPortOpenError := 'Cannot setup comm buffer';
    raise ECommsError.Create(FPortOpenError);
  end;

  // It is ok to continue.

  hCommFile := hNewCommFile;

  // purge any information in the buffer

  PurgeComm(hCommFile, PURGE_TXABORT or PURGE_RXABORT or PURGE_TXCLEAR or
    PURGE_RXCLEAR);
  FSendDataEmpty := True;

  // Setting the time-out value
  _SetCommTimeout;

  // Querying then setting the comm port configurations.
  _SetCommState;

  // Create the event that will signal the threads to close.
  hCloseEvent := CreateEvent(nil, True, False, nil);

  if hCloseEvent = 0 then
  begin
    CloseHandle(hCommFile);
    hCommFile := 0;
    FPortOpenError := 'Unable to create event';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;

  // Create the Read thread.
  try
    ReadThread := TReadThread.Create(True { suspended } );
  except
    ReadThread := nil;
    CloseHandle(hCloseEvent);
    CloseHandle(hCommFile);
    hCommFile := 0;
    FPortOpenError := 'Unable to create read thread';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;
  ReadThread.hCommFile := hCommFile;
  ReadThread.hCloseEvent := hCloseEvent;
  ReadThread.hComm32Window := FHWnd;

  // ..........................................
  ReadThread.FInputLen := FInputLen;
  ReadThread.FInBufferSize := FInBufferSize;
  // ..........................................

  // Comm threads should have a higher base priority than the UI thread.
  // If they don't, then any temporary priority boost the UI thread gains
  // could cause the COMM threads to loose data.
  ReadThread.Priority := tpHighest;

  // Create the Write thread.
  try
    WriteThread := TWriteThread.Create(True { suspended } );
  except
    CloseReadThread;
    WriteThread := nil;
    CloseHandle(hCloseEvent);
    CloseHandle(hCommFile);
    hCommFile := 0;
    FPortOpenError := 'Unable to create write thread';
    // raise ECommsError.Create( FPortOpenError );
    Exit;
  end;
  WriteThread.hCommFile := hCommFile;
  WriteThread.hCloseEvent := hCloseEvent;
  WriteThread.hComm32Window := FHWnd;
  WriteThread.pFSendDataEmpty := @FSendDataEmpty;

  WriteThread.Priority := tpHigher;

  ReadThread.Start;
  WriteThread.Start;

  // Everything was created ok.  Ready to go!
end; { TComm.StartComm }

//
// FUNCTION: StopComm
//
// PURPOSE: Stop and end all communication threads.
//
// PARAMETERS:
// none
//
// RETURN VALUE:
// none
//
// COMMENTS:
//
// Tries to gracefully signal all communication threads to
// close, but terminates them if it has to.
//
//
procedure TComm.StopComm;
begin
  // No need to continue if we're not communicating.
  FPortOpenError := '';
  FPortOpen := False;
  if hCommFile = 0 then
    Exit;

  // Close the threads.
  CloseReadThread;
  CloseWriteThread;

  // Not needed anymore.
  CloseHandle(hCloseEvent);

  // Now close the comm port handle.
  CloseHandle(hCommFile);
  hCommFile := 0
end; { TComm.StopComm }

//
// FUNCTION: WriteCommData(PAnsiChar, Word)
//
// PURPOSE: Send a String to the Write Thread to be written to the Comm.
//
// PARAMETERS:
// pszStringToWrite     - String to Write to Comm port.
// nSizeofStringToWrite - length of pszStringToWrite.
//
// RETURN VALUE:
// Returns TRUE if the PostMessage is successful.
// Returns FALSE if PostMessage fails or Write thread doesn't exist.
//
// COMMENTS:
//
// This is a wrapper function so that other modules don't care that
// Comm writing is done via PostMessage to a Write thread.  Note that
// using PostMessage speeds up response to the UI (very little delay to
// 'write' a string) and provides a natural buffer if the comm is slow
// (ie: the messages just pile up in the message queue).
//
// Note that it is assumed that pszStringToWrite is allocated with
// LocalAlloc, and that if WriteCommData succeeds, its the job of the
// Write thread to LocalFree it.  If WriteCommData fails, then its
// the job of the calling function to free the string.
//
//
function TComm.WriteCommData(pDataToWrite: PAnsiChar;
  dwSizeofDataToWrite: Word): Boolean;
var
  Buffer: Pointer;
begin
  if (WriteThread <> nil) and (dwSizeofDataToWrite <> 0) then
  begin
    Buffer := Pointer(LocalAlloc(LPTR, dwSizeofDataToWrite + 1));
    Move(pDataToWrite^, Buffer^, dwSizeofDataToWrite);
    if PostThreadMessage(WriteThread.ThreadID, PWM_COMMWRITE,
      WPARAM(dwSizeofDataToWrite), LPARAM(Buffer)) then
    begin
      FSendDataEmpty := False;
      Result := True;
      Exit
    end
  end;

  Result := False
end; { TComm.WriteCommData }

procedure TComm.SetOutput(Buffer: AnsiString);
begin
  FOutput := Buffer;
  WriteCommData(PAnsiChar(Buffer), Length(Buffer));
end;

procedure TComm.SetInputLen(StrLen: Word);
begin
  if StrLen = 0 then
    FInputLen := FInBufferSize
  else if StrLen <= FInBufferSize then
    FInputLen := StrLen;
  // ReadThread.FInputLen := FInputLen;
end;

procedure TComm.SetInBufferSize(StrSize: Word);
begin
  if StrSize = 0 then
    FInBufferSize := 1
  else
    FInBufferSize := StrSize;
  // ReadThread.FInBufferSize := FInBufferSize;
end;

//
// FUNCTION: GetModemState
//
// PURPOSE: Read the state of modem input pin right now
//
// PARAMETERS:
// none
//
// RETURN VALUE:
//
// A DWORD variable containing one or more of following codes:
//
// Value       Meaning
// ----------  -----------------------------------------------------------
// MS_CTS_ON   The CTS (clear-to-send) signal is on.
// MS_DSR_ON   The DSR (data-set-ready) signal is on.
// MS_RING_ON  The ring indicator signal is on.
// MS_RLSD_ON  The RLSD (receive-line-signal-detect) signal is on.
//
// If this comm have bad handle or not yet opened, the return value is 0
//
// COMMENTS:
//
// This member function calls GetCommModemStatus and return its value.
// Before calling this member function, you must have a successful
// 'StartOpen' call.
//
//
function TComm.GetModemState: DWORD;
var
  dwModemState: DWORD;
begin
  if not GetCommModemStatus(hCommFile, dwModemState) then
    Result := 0
  else
    Result := dwModemState
end;

(* **************************************************************************** *)
// TComm PROTECTED METHODS
(* **************************************************************************** *)

//
// FUNCTION: CloseReadThread
//
// PURPOSE: Close the Read Thread.
//
// PARAMETERS:
// none
//
// RETURN VALUE:
// none
//
// COMMENTS:
//
// Closes the Read thread by signaling the CloseEvent.
// Purges any outstanding reads on the comm port.
//
// Note that terminating a thread leaks memory.
// Besides the normal leak incurred, there is an event object
// that doesn't get closed.  This isn't worth worrying about
// since it shouldn't happen anyway.
//
//
procedure TComm.CloseReadThread;
begin
  // If it exists...
  if ReadThread <> nil then
  begin
    // Signal the event to close the worker threads.
    SetEvent(hCloseEvent);

    // Purge all outstanding reads
    PurgeComm(hCommFile, PURGE_RXABORT + PURGE_RXCLEAR);

    // Wait 10 seconds for it to exit.  Shouldn't happen.
    if (WaitForSingleObject(ReadThread.Handle, 10000) = WAIT_TIMEOUT) then
      ReadThread.Terminate;
    ReadThread.Free;
    ReadThread := nil
  end
end; { TComm.CloseReadThread }

//
// FUNCTION: CloseWriteThread
//
// PURPOSE: Closes the Write Thread.
//
// PARAMETERS:
// none
//
// RETURN VALUE:
// none
//
// COMMENTS:
//
// Closes the write thread by signaling the CloseEvent.
// Purges any outstanding writes on the comm port.
//
// Note that terminating a thread leaks memory.
// Besides the normal leak incurred, there is an event object
// that doesn't get closed.  This isn't worth worrying about
// since it shouldn't happen anyway.
//
//
procedure TComm.CloseWriteThread;
begin
  // If it exists...
  if WriteThread <> nil then
  begin
    // Signal the event to close the worker threads.
    SetEvent(hCloseEvent);

    // Purge all outstanding writes.
    PurgeComm(hCommFile, PURGE_TXABORT + PURGE_TXCLEAR);
    FSendDataEmpty := True;

    // Wait 10 seconds for it to exit.  Shouldn't happen.
    if WaitForSingleObject(WriteThread.Handle, 10000) = WAIT_TIMEOUT then
      WriteThread.Terminate;
    WriteThread.Free;
    WriteThread := nil
  end
end; { TComm.CloseWriteThread }

procedure TComm.ReceiveData(Buffer: PAnsiChar; BufferLength: Word);
begin
  if Assigned(FOnReceiveData) then // 如果添加了用户串口事件处理则执行
    FOnReceiveData(self, Buffer, BufferLength) // 用户串口事件处理
end;

procedure TComm.ReceiveError(EvtMask: DWORD);
begin
  if Assigned(FOnReceiveError) then
    FOnReceiveError(self, EvtMask)
end;

procedure TComm.ModemStateChange(ModemEvent: DWORD);
begin
  if Assigned(FOnModemStateChange) then
    FOnModemStateChange(self, ModemEvent)
end;

procedure TComm.RequestHangup;
begin
  if Assigned(FOnRequestHangup) then
    FOnRequestHangup(self)
end;

procedure TComm._SendDataEmpty;
begin
  if Assigned(FOnSendDataEmpty) then
    FOnSendDataEmpty(self)
end;

(* **************************************************************************** *)
// TComm PRIVATE METHODS
(* **************************************************************************** *)

procedure TComm.CommWndProc(var msg: TMessage);
begin
  case msg.msg of
    PWM_GOTCOMMDATA:
      begin
        ReceiveData(PAnsiChar(msg.LPARAM), msg.WPARAM);
        LocalFree(msg.LPARAM)
      end;
    PWM_RECEIVEERROR:
      ReceiveError(msg.LPARAM);
    PWM_MODEMSTATECHANGE:
      ModemStateChange(msg.LPARAM);
    PWM_REQUESTHANGUP:
      RequestHangup;
    PWM_SENDDATAEMPTY:
      _SendDataEmpty
  end
end;

procedure TComm._SetCommState;
var
  dcb: Tdcb;
  commprop: TCommProp;
  fdwEvtMask: DWORD;
begin
  // Configure the comm settings.
  // NOTE: Most Comm settings can be set through TAPI, but this means that
  // the CommFile will have to be passed to this component.

  GetCommState(hCommFile, dcb);
  GetCommProperties(hCommFile, commprop);
  GetCommMask(hCommFile, fdwEvtMask);

  // fAbortOnError is the only DCB dependancy in TapiComm.
  // Can't guarentee that the SP will set this to what we expect.
  { dcb.fAbortOnError := False; NOT VALID }

  dcb.BaudRate := FBaudRate;

  dcb.Flags := 1; // Enable fBinary

  if FParityCheck then
    dcb.Flags := dcb.Flags or 2; // Enable parity check

  // setup hardware flow control

  if FOutx_CtsFlow then
    dcb.Flags := dcb.Flags or 4;

  if FOutx_DsrFlow then
    dcb.Flags := dcb.Flags or 8;

  if FDtrControl = DtrEnable then
    dcb.Flags := dcb.Flags or $10
  else if FDtrControl = DtrHandshake then
    dcb.Flags := dcb.Flags or $20;

  if FDsrSensitivity then
    dcb.Flags := dcb.Flags or $40;

  if FTxContinueOnXoff then
    dcb.Flags := dcb.Flags or $80;

  if FOutx_XonXoffFlow then
    dcb.Flags := dcb.Flags or $100;

  if FInx_XonXoffFlow then
    dcb.Flags := dcb.Flags or $200;

  if FReplaceWhenParityError then
    dcb.Flags := dcb.Flags or $400;

  if FIgnoreNullChar then
    dcb.Flags := dcb.Flags or $800;

  if FRtsControl = RtsEnable then
    dcb.Flags := dcb.Flags or $1000
  else if FRtsControl = RtsHandshake then
    dcb.Flags := dcb.Flags or $2000
  else if FRtsControl = RtsTransmissionAvailable then
    dcb.Flags := dcb.Flags or $3000;

  dcb.XonLim := FXonLimit;
  dcb.XoffLim := FXoffLimit;

  dcb.ByteSize := Ord(FByteSize) + 5;
  dcb.Parity := Ord(FParity);
  dcb.StopBits := Ord(FStopBits);

  dcb.XonChar := FXonChar;
  dcb.XoffChar := FXoffChar;

  dcb.ErrorChar := FReplacedChar;

  SetCommState(hCommFile, dcb)
end;

procedure TComm._SetCommTimeout;
var
  commtimeouts: TCommTimeouts;
begin
  GetCommTimeouts(hCommFile, commtimeouts);

  // The CommTimeout numbers will very likely change if you are
  // coding to meet some kind of specification where
  // you need to reply within a certain amount of time after
  // recieving the last byte.  However,  If 1/4th of a second
  // goes by between recieving two characters, its a good
  // indication that the transmitting end has finished, even
  // assuming a 1200 baud modem.

  commtimeouts.ReadIntervalTimeout := FReadIntervalTimeout;
  commtimeouts.ReadTotalTimeoutMultiplier := FReadTotalTimeoutMultiplier;
  commtimeouts.ReadTotalTimeoutConstant := FReadTotalTimeoutConstant;
  commtimeouts.WriteTotalTimeoutMultiplier := FWriteTotalTimeoutMultiplier;
  commtimeouts.WriteTotalTimeoutConstant := FWriteTotalTimeoutConstant;

  SetCommTimeouts(hCommFile, commtimeouts);
end;

procedure TComm.SetPortOpen(b: Boolean); // 打开端口
begin
  if b = True then
    StartComm // 打开端口
  else
    StopComm; // 关闭端口
end;

function TComm.GetPortOpen: Boolean; // 打开端口
begin
  if hCommFile = 0 then
  begin
    FPortOpen := False;
    Result := False;
  end
  else
  begin
    FPortOpen := True;
    Result := True;
  end;
end;

procedure TComm.SetCommPort(CommPort: BYTE);
begin
  if (CommPort <> 0) and (CommPort <> FCommPort) then
  begin
    FCommPort := CommPort;
    FCommName := 'COM' + inttostr(CommPort);
  end;
end;

procedure TComm.SetBaudRate(Rate: DWORD);
begin
  if Rate = FBaudRate then
    Exit;

  FBaudRate := Rate;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetParityCheck(b: Boolean);
begin
  if b = FParityCheck then
    Exit;

  FParityCheck := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetOutx_CtsFlow(b: Boolean);
begin
  if b = FOutx_CtsFlow then
    Exit;

  FOutx_CtsFlow := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetOutx_DsrFlow(b: Boolean);
begin
  if b = FOutx_DsrFlow then
    Exit;

  FOutx_DsrFlow := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetDtrControl(c: TDtrControl);
begin
  if c = FDtrControl then
    Exit;

  FDtrControl := c;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetDtrRtsControl(DtrControl: TDtrControl;
  RtsControl: TRtsControl);
begin
  if ((DtrControl = FDtrControl) and (RtsControl = FRtsControl)) then
    Exit;
  FDtrControl := DtrControl;
  FRtsControl := RtsControl;
  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetDsrSensitivity(b: Boolean);
begin
  if b = FDsrSensitivity then
    Exit;

  FDsrSensitivity := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetTxContinueOnXoff(b: Boolean);
begin
  if b = FTxContinueOnXoff then
    Exit;

  FTxContinueOnXoff := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetOutx_XonXoffFlow(b: Boolean);
begin
  if b = FOutx_XonXoffFlow then
    Exit;

  FOutx_XonXoffFlow := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetInx_XonXoffFlow(b: Boolean);
begin
  if b = FInx_XonXoffFlow then
    Exit;

  FInx_XonXoffFlow := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetReplaceWhenParityError(b: Boolean);
begin
  if b = FReplaceWhenParityError then
    Exit;

  FReplaceWhenParityError := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetIgnoreNullChar(b: Boolean);
begin
  if b = FIgnoreNullChar then
    Exit;

  FIgnoreNullChar := b;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetRtsControl(c: TRtsControl);
begin
  if c = FRtsControl then
    Exit;

  FRtsControl := c;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetXonLimit(Limit: Word);
begin
  if Limit = FXonLimit then
    Exit;

  FXonLimit := Limit;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetXoffLimit(Limit: Word);
begin
  if Limit = FXoffLimit then
    Exit;

  FXoffLimit := Limit;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetByteSize(Size: TByteSize);
begin
  if Size = FByteSize then
    Exit;

  FByteSize := Size;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetParity(p: TParity);
begin
  if p = FParity then
    Exit;

  FParity := p;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetStopBits(Bits: TStopBits);
begin
  if Bits = FStopBits then
    Exit;

  FStopBits := Bits;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetXonChar(c: AnsiChar);
begin
  if c = FXonChar then
    Exit;

  FXonChar := c;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetXoffChar(c: AnsiChar);
begin
  if c = FXoffChar then
    Exit;

  FXoffChar := c;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetReplacedChar(c: AnsiChar);
begin
  if c = FReplacedChar then
    Exit;

  FReplacedChar := c;

  if hCommFile <> 0 then
    _SetCommState
end;

procedure TComm.SetReadIntervalTimeout(v: DWORD);
begin
  if v = FReadIntervalTimeout then
    Exit;

  FReadIntervalTimeout := v;

  if hCommFile <> 0 then
    _SetCommTimeout
end;

procedure TComm.SetReadTotalTimeoutMultiplier(v: DWORD);
begin
  if v = FReadTotalTimeoutMultiplier then
    Exit;

  FReadTotalTimeoutMultiplier := v;

  if hCommFile <> 0 then
    _SetCommTimeout
end;

procedure TComm.SetReadTotalTimeoutConstant(v: DWORD);
begin
  if v = FReadTotalTimeoutConstant then
    Exit;

  FReadTotalTimeoutConstant := v;

  if hCommFile <> 0 then
    _SetCommTimeout
end;

procedure TComm.SetWriteTotalTimeoutMultiplier(v: DWORD);
begin
  if v = FWriteTotalTimeoutMultiplier then
    Exit;

  FWriteTotalTimeoutMultiplier := v;

  if hCommFile <> 0 then
    _SetCommTimeout
end;

procedure TComm.SetWriteTotalTimeoutConstant(v: DWORD);
begin
  if v = FWriteTotalTimeoutConstant then
    Exit;

  FWriteTotalTimeoutConstant := v;

  if hCommFile <> 0 then
    _SetCommTimeout
end;

(* **************************************************************************** *)
// READ THREAD
(* **************************************************************************** *)

//
// PROCEDURE: TReadThread.Execute
//
// PURPOSE: This is the starting point for the Read Thread.
//
// PARAMETERS:
// None.
//
// RETURN VALUE:
// None.
//
// COMMENTS:
//
// The Read Thread uses overlapped ReadFile and sends any data
// read from the comm port to the Comm32Window.  This is
// eventually done through a PostMessage so that the Read Thread
// is never away from the comm port very long.  This also provides
// natural desynchronization between the Read thread and the UI.
//
// If the CloseEvent object is signaled, the Read Thread exits.
//
// Separating the Read and Write threads is natural for a application
// where there is no need for synchronization between
// reading and writing.  However, if there is such a need (for example,
// most file transfer algorithms synchronize the reading and writing),
// then it would make a lot more sense to have a single thread to handle
// both reading and writing.
//
//
procedure TReadThread.Execute;
var
  szInputBuffer: array of Char; // 新方法灵活
  // szInputBuffer: array[0..INPUTBUFFERSIZE-1] of Char;//老方法不灵活
  nNumberOfBytesRead: DWORD;

  HandlesToWaitFor: array [0 .. 2] of THandle;
  dwHandleSignaled: DWORD;

  fdwEvtMask: DWORD;

  // Needed for overlapped I/O (ReadFile)
  overlappedRead: TOverlapped;

  // Needed for overlapped Comm Event handling.
  overlappedCommEvent: TOverlapped;
label
  EndReadThread;
begin
  SetLength(szInputBuffer, FInBufferSize); // 新方法数组大小可变
  FillChar(overlappedRead, Sizeof(overlappedRead), 0);
  FillChar(overlappedCommEvent, Sizeof(overlappedCommEvent), 0);

  // Lets put an event in the Read overlapped structure.
  overlappedRead.hEvent := CreateEvent(nil, True, True, nil);
  if overlappedRead.hEvent = 0 then
  begin
    PostHangupCall;
    goto EndReadThread
  end;

  // And an event for the CommEvent overlapped structure.
  overlappedCommEvent.hEvent := CreateEvent(nil, True, True, nil);
  if overlappedCommEvent.hEvent = 0 then
  begin
    PostHangupCall();
    goto EndReadThread
  end;

  // We will be waiting on these objects.
  HandlesToWaitFor[0] := hCloseEvent;
  HandlesToWaitFor[1] := overlappedCommEvent.hEvent;
  HandlesToWaitFor[2] := overlappedRead.hEvent;

  // Setup CommEvent handling.

  // Set the comm mask so we receive error signals.
  if not SetCommMask(hCommFile, EV_ERR or EV_RLSD or EV_RING) then
  begin
    PostHangupCall;
    goto EndReadThread
  end;

  // Start waiting for CommEvents (Errors)
  if not SetupCommEvent(@overlappedCommEvent, fdwEvtMask) then
    goto EndReadThread;

  // Start waiting for Read events.
  if not SetupReadEvent(@overlappedRead,
    // PAnsiChar(szInputBuffer),//新方法不能用@szInputBuffer
    PAnsiChar(szInputBuffer), // 新方法不能用@szInputBuffer
    // PAnsiChar(AnsiString(Str))
    FInputLen, nNumberOfBytesRead) then
    goto EndReadThread;

  // Keep looping until we break out.
  while True do
  begin
    // Wait until some event occurs (data to read; error; stopping).
    dwHandleSignaled := WaitForMultipleObjects(3, @HandlesToWaitFor, False,
      INFINITE);

    // Which event occured?
    case dwHandleSignaled of
      WAIT_OBJECT_0: // Signal to end the thread.
        begin
          // Time to exit.
          goto EndReadThread
        end;

      WAIT_OBJECT_0 + 1: // CommEvent signaled.
        begin
          // Handle the CommEvent.
          if not HandleCommEvent(@overlappedCommEvent, fdwEvtMask, True) then
            goto EndReadThread;

          // Start waiting for the next CommEvent.
          if not SetupCommEvent(@overlappedCommEvent, fdwEvtMask) then
            goto EndReadThread
            { break;?? }
        end;

      WAIT_OBJECT_0 + 2: // Read Event signaled.
        begin
          // Get the new data!
          if not HandleReadEvent(@overlappedRead, PAnsiChar(szInputBuffer),
            // 新方法不能用@szInputBuffer
            FInputLen, nNumberOfBytesRead) then
            goto EndReadThread;

          // Wait for more new data.
          if not SetupReadEvent(@overlappedRead, PAnsiChar(szInputBuffer),
            // 新方法不能用@szInputBuffer
            FInputLen, nNumberOfBytesRead) then
            goto EndReadThread
            { break; }
        end;

      WAIT_FAILED: // Wait failed.  Shouldn't happen.
        begin
          PostHangupCall;
          goto EndReadThread
        end
    else // This case should never occur.
      begin
        PostHangupCall;
        goto EndReadThread
      end
    end { case dwHandleSignaled }
  end; { while True }

  // Time to clean up Read Thread.
EndReadThread:

  PurgeComm(hCommFile, PURGE_RXABORT + PURGE_RXCLEAR);
  CloseHandle(overlappedRead.hEvent);
  CloseHandle(overlappedCommEvent.hEvent)
end; { TReadThread.Execute }

//
// FUNCTION: SetupReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//
// PURPOSE: Sets up an overlapped ReadFile
//
// PARAMETERS:
// lpOverlappedRead      - address of overlapped structure to use.
// lpszInputBuffer       - Buffer to place incoming bytes.
// dwSizeofBuffer        - size of lpszInputBuffer.
// lpnNumberOfBytesRead  - address of DWORD to place the number of read bytes.
//
// RETURN VALUE:
// TRUE if able to successfully setup the ReadFile.  FALSE if there
// was a failure setting up or if the CloseEvent object was signaled.
//
// COMMENTS:
//
// This function is a helper function for the Read Thread.  This
// function sets up the overlapped ReadFile so that it can later
// be waited on (or more appropriatly, so the event in the overlapped
// structure can be waited upon).  If there is data waiting, it is
// handled and the next ReadFile is initiated.
// Another possible reason for returning FALSE is if the comm port
// is closed by the service provider.
//
//
//
function TReadThread.SetupReadEvent(lpOverlappedRead: POverlapped;
  lpszInputBuffer: LPSTR; dwSizeofBuffer: DWORD;
  var lpnNumberOfBytesRead: DWORD): Boolean;
var
  dwLastError: DWORD;
label
  StartSetupReadEvent;
begin
  Result := False;

StartSetupReadEvent:

  // Make sure the CloseEvent hasn't been signaled yet.
  // Check is needed because this function is potentially recursive.
  if WAIT_TIMEOUT <> WaitForSingleObject(hCloseEvent, 0) then
    Exit;

  // Start the overlapped ReadFile.
  if ReadFile(hCommFile, lpszInputBuffer^, dwSizeofBuffer, lpnNumberOfBytesRead,
    lpOverlappedRead) then
  begin
    // This would only happen if there was data waiting to be read.

    // Handle the data.
    if not HandleReadData(lpszInputBuffer, lpnNumberOfBytesRead) then
      Exit;

    // Start waiting for more data.
    goto StartSetupReadEvent
  end;

  // ReadFile failed.  Expected because of overlapped I/O.
  dwLastError := GetLastError;

  // LastError was ERROR_IO_PENDING, as expected.
  if dwLastError = ERROR_IO_PENDING then
  begin
    Result := True;
    Exit
  end;

  // Its possible for this error to occur if the
  // service provider has closed the port.  Time to end.
  if dwLastError = ERROR_INVALID_HANDLE then
    Exit;

  // Unexpected error come here. No idea what could cause this to happen.
  PostHangupCall
end; { TReadThread.SetupReadEvent }

//
// FUNCTION: HandleReadData(LPCSTR, DWORD)
//
// PURPOSE: Deals with data after its been read from the comm file.
//
// PARAMETERS:
// lpszInputBuffer  - Buffer to place incoming bytes.
// dwSizeofBuffer   - size of lpszInputBuffer.
//
// RETURN VALUE:
// TRUE if able to successfully handle the data.
// FALSE if unable to allocate memory or handle the data.
//
// COMMENTS:
//
// This function is yet another helper function for the Read Thread.
// It LocalAlloc()s a buffer, copies the new data to this buffer and
// calls PostWriteToDisplayCtl to let the EditCtls module deal with
// the data.  Its assumed that PostWriteToDisplayCtl posts the message
// rather than dealing with it right away so that the Read Thread
// is free to get right back to waiting for data.  Its also assumed
// that the EditCtls module is responsible for LocalFree()ing the
// pointer that is passed on.
//
//
function TReadThread.HandleReadData(lpszInputBuffer: LPCSTR;
  dwSizeofBuffer: DWORD): Boolean;
var
  lpszPostedBytes: LPSTR;
begin
  Result := False;

  // If we got data and didn't just time out empty...
  if dwSizeofBuffer <> 0 then
  begin
    // Do something with the bytes read.

    lpszPostedBytes := PAnsiChar(LocalAlloc(LPTR, dwSizeofBuffer + 1));

    if lpszPostedBytes = nil { NULL } then
    begin
      // Out of memory

      PostHangupCall;
      Exit
    end;

    Move(lpszInputBuffer^, lpszPostedBytes^, dwSizeofBuffer);
    lpszPostedBytes[dwSizeofBuffer] := #0;

    Result := ReceiveData(lpszPostedBytes, dwSizeofBuffer)
  end
end; { TReadThread.HandleReadData }

//
// FUNCTION: HandleReadEvent(LPOVERLAPPED, LPSTR, DWORD, LPDWORD)
//
// PURPOSE: Retrieves and handles data when there is data ready.
//
// PARAMETERS:
// lpOverlappedRead      - address of overlapped structure to use.
// lpszInputBuffer       - Buffer to place incoming bytes.
// dwSizeofBuffer        - size of lpszInputBuffer.
// lpnNumberOfBytesRead  - address of DWORD to place the number of read bytes.
//
// RETURN VALUE:
// TRUE if able to successfully retrieve and handle the available data.
// FALSE if unable to retrieve or handle the data.
//
// COMMENTS:
//
// This function is another helper function for the Read Thread.  This
// is the function that is called when there is data available after
// an overlapped ReadFile has been setup.  It retrieves the data and
// handles it.
//
//
function TReadThread.HandleReadEvent(lpOverlappedRead: POverlapped;
  lpszInputBuffer: LPSTR; dwSizeofBuffer: DWORD;
  var lpnNumberOfBytesRead: DWORD): Boolean;
var
  dwLastError: DWORD;
begin
  Result := False;

  if GetOverlappedResult(hCommFile, lpOverlappedRead^, lpnNumberOfBytesRead,
    False) then
  begin
    Result := HandleReadData(lpszInputBuffer, lpnNumberOfBytesRead);
    Exit
  end;

  // Error in GetOverlappedResult; handle it.

  dwLastError := GetLastError;

  // Its possible for this error to occur if the
  // service provider has closed the port.  Time to end.
  if dwLastError = ERROR_INVALID_HANDLE then
    Exit;

  // Unexpected error come here. No idea what could cause this to happen.
  PostHangupCall
end; { TReadThread.HandleReadEvent }

//
// FUNCTION: SetupCommEvent(LPOVERLAPPED, LPDWORD)
//
// PURPOSE: Sets up the overlapped WaitCommEvent call.
//
// PARAMETERS:
// lpOverlappedCommEvent - Pointer to the overlapped structure to use.
// lpfdwEvtMask          - Pointer to DWORD to received Event data.
//
// RETURN VALUE:
// TRUE if able to successfully setup the WaitCommEvent.
// FALSE if unable to setup WaitCommEvent, unable to handle
// an existing outstanding event or if the CloseEvent has been signaled.
//
// COMMENTS:
//
// This function is a helper function for the Read Thread that sets up
// the WaitCommEvent so we can deal with comm events (like Comm errors)
// if they occur.
//
//
function TReadThread.SetupCommEvent(lpOverlappedCommEvent: POverlapped;
  var lpfdwEvtMask: DWORD): Boolean;
var
  dwLastError: DWORD;
label
  StartSetupCommEvent;
begin
  Result := False;

StartSetupCommEvent:

  // Make sure the CloseEvent hasn't been signaled yet.
  // Check is needed because this function is potentially recursive.
  if WAIT_TIMEOUT <> WaitForSingleObject(hCloseEvent, 0) then
    Exit;

  // Start waiting for Comm Errors.
  if WaitCommEvent(hCommFile, lpfdwEvtMask, lpOverlappedCommEvent) then
  begin
    // This could happen if there was an error waiting on the
    // comm port.  Lets try and handle it.

    if not HandleCommEvent(nil, lpfdwEvtMask, False) then
    begin
      { ??? GetOverlappedResult does not handle "NIL" as defined by Borland }
      Exit
    end;

    // What could cause infinite recursion at this point?
    goto StartSetupCommEvent
  end;

  // We expect ERROR_IO_PENDING returned from WaitCommEvent
  // because we are waiting with an overlapped structure.

  dwLastError := GetLastError;

  // LastError was ERROR_IO_PENDING, as expected.
  if dwLastError = ERROR_IO_PENDING then
  begin
    Result := True;
    Exit
  end;

  // Its possible for this error to occur if the
  // service provider has closed the port.  Time to end.
  if dwLastError = ERROR_INVALID_HANDLE then
    Exit;

  // Unexpected error. No idea what could cause this to happen.
  PostHangupCall
end; { TReadThread.SetupCommEvent }

//
// FUNCTION: HandleCommEvent(LPOVERLAPPED, LPDWORD, BOOL)
//
// PURPOSE: Handle an outstanding Comm Event.
//
// PARAMETERS:
// lpOverlappedCommEvent - Pointer to the overlapped structure to use.
// lpfdwEvtMask          - Pointer to DWORD to received Event data.
// fRetrieveEvent       - Flag to signal if the event needs to be
// retrieved, or has already been retrieved.
//
// RETURN VALUE:
// TRUE if able to handle a Comm Event.
// FALSE if unable to setup WaitCommEvent, unable to handle
// an existing outstanding event or if the CloseEvent has been signaled.
//
// COMMENTS:
//
// This function is a helper function for the Read Thread that (if
// fRetrieveEvent == TRUE) retrieves an outstanding CommEvent and
// deals with it.  The only event that should occur is an EV_ERR event,
// signalling that there has been an error on the comm port.
//
// Normally, comm errors would not be put into the normal data stream
// as this sample is demonstrating.  Putting it in a status bar would
// be more appropriate for a real application.
//
//
function TReadThread.HandleCommEvent(lpOverlappedCommEvent: POverlapped;
  var lpfdwEvtMask: DWORD; fRetrieveEvent: Boolean): Boolean;
var
  dwDummy: DWORD;
  dwErrors: DWORD;
  dwLastError: DWORD;
  dwModemEvent: DWORD;
begin
  Result := False;

  // If this fails, it could be because the file was closed (and I/O is
  // finished) or because the overlapped I/O is still in progress.  In
  // either case (or any others) its a bug and return FALSE.
  if fRetrieveEvent then
  begin
    if not GetOverlappedResult(hCommFile, lpOverlappedCommEvent^, dwDummy, False)
    then
    begin
      dwLastError := GetLastError;

      // Its possible for this error to occur if the
      // service provider has closed the port.  Time to end.
      if dwLastError = ERROR_INVALID_HANDLE then
        Exit;

      PostHangupCall;
      Exit
    end
  end;

  // Was the event an error?
  if (lpfdwEvtMask and EV_ERR) <> 0 then
  begin
    // Which error was it?
    if not ClearCommError(hCommFile, dwErrors, nil) then
    begin
      dwLastError := GetLastError;

      // Its possible for this error to occur if the
      // service provider has closed the port.  Time to end.
      if dwLastError = ERROR_INVALID_HANDLE then
        Exit;

      PostHangupCall;
      Exit
    end;

    // Its possible that multiple errors occured and were handled
    // in the last ClearCommError.  Because all errors were signaled
    // individually, but cleared all at once, pending comm events
    // can yield EV_ERR while dwErrors equals 0.  Ignore this event.

    if not ReceiveError(dwErrors) then
      Exit;

    Result := True
  end;

  dwModemEvent := 0;

  if ((lpfdwEvtMask and EV_RLSD) <> 0) then
    dwModemEvent := ME_RLSD;
  if ((lpfdwEvtMask and EV_RING) <> 0) then
    dwModemEvent := dwModemEvent or ME_RING;

  if dwModemEvent <> 0 then
  begin
    if not ModemStateChange(dwModemEvent) then
    begin
      Result := False;
      Exit
    end;

    Result := True
  end;

  if ((lpfdwEvtMask and EV_ERR) = 0) and (dwModemEvent = 0) then
  begin
    // Should not have gotten here.
    PostHangupCall
  end
end; { TReadThread.HandleCommEvent }

function TReadThread.ReceiveData(lpNewString: LPSTR;
  dwSizeofNewString: DWORD): BOOL;
begin
  Result := False;

  if not PostMessage(hComm32Window, PWM_GOTCOMMDATA, WPARAM(dwSizeofNewString),
    LPARAM(lpNewString)) then
    PostHangupCall
  else
    Result := True
end;

function TReadThread.ReceiveError(EvtMask: DWORD): BOOL;
begin
  Result := False;

  if not PostMessage(hComm32Window, PWM_RECEIVEERROR, 0, LPARAM(EvtMask)) then
    PostHangupCall
  else
    Result := True
end;

function TReadThread.ModemStateChange(ModemEvent: DWORD): BOOL;
begin
  Result := False;

  if not PostMessage(hComm32Window, PWM_MODEMSTATECHANGE, 0, LPARAM(ModemEvent))
  then
    PostHangupCall
  else
    Result := True
end;

procedure TReadThread.PostHangupCall;
begin
  PostMessage(hComm32Window, PWM_REQUESTHANGUP, 0, 0)
end;

(* **************************************************************************** *)
// WRITE THREAD
(* **************************************************************************** *)

//
// PROCEDURE: TWriteThread.Execute
//
// PURPOSE: The starting point for the Write thread.
//
// PARAMETERS:
// lpvParam - unused.
//
// RETURN VALUE:
// DWORD - unused.
//
// COMMENTS:
//
// The Write thread uses a PeekMessage loop to wait for a string to write,
// and when it gets one, it writes it to the Comm port.  If the CloseEvent
// object is signaled, then it exits.  The use of messages to tell the
// Write thread what to write provides a natural desynchronization between
// the UI and the Write thread.
//
//
procedure TWriteThread.Execute;
var
  msg: TMsg;
  dwHandleSignaled: DWORD;
  overlappedWrite: TOverlapped;
  CompleteOneWriteRequire: Boolean;
label
  EndWriteThread;
begin
  // Needed for overlapped I/O.
  FillChar(overlappedWrite, Sizeof(overlappedWrite), 0); { 0, 0, 0, 0, NULL }

  overlappedWrite.hEvent := CreateEvent(nil, True, True, nil);
  if overlappedWrite.hEvent = 0 then
  begin
    PostHangupCall;
    goto EndWriteThread
  end;

  CompleteOneWriteRequire := True;

  // This is the main loop.  Loop until we break out.
  while True do
  begin
    if not PeekMessage(msg, 0, 0, 0, PM_REMOVE) then
    begin
      // If there are no messages pending, wait for a message or
      // the CloseEvent.

      pFSendDataEmpty^ := True;

      if CompleteOneWriteRequire then
      begin
        if not PostMessage(hComm32Window, PWM_SENDDATAEMPTY, 0, 0) then
        begin
          PostHangupCall;
          goto EndWriteThread
        end
      end;

      CompleteOneWriteRequire := False;

      dwHandleSignaled := MsgWaitForMultipleObjects(1, hCloseEvent, False,
        INFINITE, QS_ALLINPUT);

      case dwHandleSignaled of
        WAIT_OBJECT_0: // CloseEvent signaled!
          begin
            // Time to exit.
            goto EndWriteThread
          end;

        WAIT_OBJECT_0 + 1: // New message was received.
          begin
            // Get the message that woke us up by looping again.
            Continue
          end;

        WAIT_FAILED: // Wait failed.  Shouldn't happen.
          begin
            PostHangupCall;
            goto EndWriteThread
          end

      else // This case should never occur.
        begin
          PostHangupCall;
          goto EndWriteThread
        end
      end
    end;

    // Make sure the CloseEvent isn't signaled while retrieving messages.
    if WAIT_TIMEOUT <> WaitForSingleObject(hCloseEvent, 0) then
      goto EndWriteThread;

    // Process the message.
    // This could happen if a dialog is created on this thread.
    // This doesn't occur in this sample, but might if modified.
    if msg.hwnd <> 0 { NULL } then
    begin
      TranslateMessage(msg);
      DispatchMessage(msg);
      Continue
    end;

    // Handle the message.
    case msg.message of
      PWM_COMMWRITE: // New string to write to Comm port.
        begin
          // Write the string to the comm port.  HandleWriteData
          // does not return until the whole string has been written,
          // an error occurs or until the CloseEvent is signaled.
          if not HandleWriteData(@overlappedWrite, PAnsiChar(msg.LPARAM),
            DWORD(msg.WPARAM)) then
          begin
            // If it failed, either we got a signal to end or there
            // really was a failure.

            LocalFree(HLOCAL(msg.LPARAM));
            goto EndWriteThread
          end;

          CompleteOneWriteRequire := True;
          // Data was sent in a LocalAlloc()d buffer.  Must free it.
          LocalFree(HLOCAL(msg.LPARAM))
        end
    end
  end; { main loop }

  // Thats the end.  Now clean up.
EndWriteThread:

  PurgeComm(hCommFile, PURGE_TXABORT + PURGE_TXCLEAR);
  pFSendDataEmpty^ := True;
  CloseHandle(overlappedWrite.hEvent)
end; { TWriteThread.Execute }

//
// FUNCTION: HandleWriteData(LPOVERLAPPED, LPCSTR, DWORD)
//
// PURPOSE: Writes a given string to the comm file handle.
//
// PARAMETERS:
// lpOverlappedWrite  - Overlapped structure to use in WriteFile
// pDataToWrite       - String to write.
// dwNumberOfBytesToWrite - Length of String to write.
//
// RETURN VALUE:
// TRUE if all bytes were written.  False if there was a failure to
// write the whole string.
//
// COMMENTS:
//
// This function is a helper function for the Write Thread.  It
// is this call that actually writes a string to the comm file.
// Note that this call blocks and waits for the Write to complete
// or for the CloseEvent object to signal that the thread should end.
// Another possible reason for returning FALSE is if the comm port
// is closed by the service provider.
//
//
function TWriteThread.HandleWriteData(lpOverlappedWrite: POverlapped;
  pDataToWrite: PAnsiChar; dwNumberOfBytesToWrite: DWORD): Boolean;
var
  dwLastError,

    dwNumberOfBytesWritten, dwWhereToStartWriting,

    dwHandleSignaled: DWORD;
  HandlesToWaitFor: array [0 .. 1] of THandle;
begin
  Result := False;

  dwNumberOfBytesWritten := 0;
  dwWhereToStartWriting := 0; // Start at the beginning.

  HandlesToWaitFor[0] := hCloseEvent;
  HandlesToWaitFor[1] := lpOverlappedWrite^.hEvent;

  // Keep looping until all characters have been written.
  repeat
    // Start the overlapped I/O.
    if not WriteFile(hCommFile, pDataToWrite[dwWhereToStartWriting],
      dwNumberOfBytesToWrite, dwNumberOfBytesWritten, lpOverlappedWrite) then
    begin
      // WriteFile failed.  Expected; lets handle it.
      dwLastError := GetLastError;

      // Its possible for this error to occur if the
      // service provider has closed the port.  Time to end.
      if dwLastError = ERROR_INVALID_HANDLE then
        Exit;

      // Unexpected error.  No idea what.
      if dwLastError <> ERROR_IO_PENDING then
      begin
        PostHangupCall;
        Exit
      end;

      // This is the expected ERROR_IO_PENDING case.

      // Wait for either overlapped I/O completion,
      // or for the CloseEvent to get signaled.
      dwHandleSignaled := WaitForMultipleObjects(2, @HandlesToWaitFor, False,
        INFINITE);

      case dwHandleSignaled of
        WAIT_OBJECT_0: // CloseEvent signaled!
          begin
            // Time to exit.
            Exit
          end;

        WAIT_OBJECT_0 + 1: // Wait finished.
          begin
            // Time to get the results of the WriteFile
            if not GetOverlappedResult(hCommFile, lpOverlappedWrite^,
              dwNumberOfBytesWritten, True) then
            begin
              dwLastError := GetLastError;

              // Its possible for this error to occur if the
              // service provider has closed the port.
              if dwLastError = ERROR_INVALID_HANDLE then
                Exit;

              // No idea what could cause another error.
              PostHangupCall;
              Exit
            end
          end;

        WAIT_FAILED: // Wait failed.  Shouldn't happen.
          begin
            PostHangupCall;
            Exit
          end

      else // This case should never occur.
        begin
          PostHangupCall;
          Exit
        end
      end { case }
    end; { WriteFile failure }

    // Some data was written.  Make sure it all got written.

    Dec(dwNumberOfBytesToWrite, dwNumberOfBytesWritten);
    Inc(dwWhereToStartWriting, dwNumberOfBytesWritten)
  until (dwNumberOfBytesToWrite <= 0); // Write the whole thing!

  // Wrote the whole string.
  Result := True
end; { TWriteThread.HandleWriteData }

procedure TWriteThread.PostHangupCall;
begin
  PostMessage(hComm32Window, PWM_REQUESTHANGUP, 0, 0)
end;

procedure Register;
begin
  RegisterComponents('System', [TComm])
end;

end.
