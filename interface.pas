unit Unit1;

interface

uses
  Windows, Messages, SysUtils, Variants, Classes, Graphics, Controls, Forms,
  Dialogs, StdCtrls;

type
  TForm1 = class(TForm)
    Button1: TButton;
    Button2: TButton;
    procedure Button1Click(Sender: TObject);
    procedure Button2Click(Sender: TObject);
  private
    { Private declarations }
  public
    { Public declarations }
  end;

var
  Form1: TForm1;

implementation

{$R *.dfm}

procedure TForm1.Button1Click(Sender: TObject);
var _curr_process:DWORD;
    p_handle: THANDLE;
begin
// Убиваем родительский процесс
_curr_process:=GetCurrentProcessId();
p_handle:=OpenProcess(PROCESS_ALL_ACCESS,false,_curr_process);
TerminateProcess(p_handle,4);
end;

var dw:DWORD;  // PID текущего процесса

procedure drawGroup(h:HWND);
var canvas:TBitMap;
rect:TRect;
x,y:integer;
i: Integer;
begin
// рисуем 10 квадратов
for i := 1 to 10 do
begin
canvas:=TBitMap.Create();
canvas.Canvas.Handle:=GetDC(h);  // устанавливаем дескриптор контекста
randomize;
canvas.Canvas.Brush.Color:=rgb(random(255),random(255),random(255));
GetWindowRect(h,rect);
x:=random(rect.Right-rect.Left-25);
y:=random(rect.Bottom-rect.Top-25);
canvas.Canvas.Rectangle(x,y,x+25,y+25);
end;
end;

function func(h:HWND):BOOL;  stdcall;
var pid:DWORD;
begin
GetWindowThreadProcessId(h,pid);
// запускаем функцию рисования в окне, если оно принадлежит нужному процессу:
if(pid=dw) then drawGroup(h);
result:=true;
end;

procedure TForm1.Button2Click(Sender: TObject);
begin
// Запускаем функцию рисования квадратов
// ( в перечислителе окон системы )
dw:=GetCurrentProcessId();
EnumWindows(@func,0);    // перечисляем окна
end;

end.
