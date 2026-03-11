program Project1;

uses Windows,
   Registry,
   Dialogs,
   ShellApi,
   FileCtrl,
   ShlObj,
   SysUtils,
   Forms,
   Classes,
   Unit1 in 'Unit1.pas' {Form1};

{$R *.RES}

procedure AddToRecent(s: string);
begin
   shAddToRecentDocs(SHARD_PATH, PChar(s));
end;

var
   recentsFolder: string;
   programsFolder: string;
   desktopFolder: string;
   pathInfoFile: string;
   exeName: string;
   pathInfo: string;
   dialogTitle: string;
   dialogInfo: string;
   newGroup: string;
   foo: string;
   scriptFile: string;
   linkText, linkFile: string;
   longPath: string;
   userCancel: boolean;
   reg: TRegistry;
   fich, pathfich: TextFile;

begin
   Application.Initialize;
   Application.Title := 'DIV Games Studio 2';
   Application.CreateForm(TForm1, Form1);
   reg := TRegistry.Create;
   try
      reg.RootKey := HKey_Current_User;
      reg.OpenKey('Software\Microsoft\Windows\CurrentVersion\Explorer\Shell Folders', False);
      programsFolder := reg.ReadString('Programs') ;
      recentsFolder := reg.ReadString('Recent') ;
      desktopFolder := reg.ReadString('Desktop') ;
   finally
      reg.Free;
   end;

   scriptFile := ExtractFilePath(Application.ExeName) + 'data\system\group.dat';
   AssignFile(fich, scriptFile);
   Reset(fich);
   ReadLn(fich, foo);
   Readln(fich, pathInfoFile);
   ReadLn(fich, foo);
   Readln(fich, exeName);
   ReadLn(fich, foo);
   Readln(fich, dialogTitle);
   ReadLn(fich, foo);
   dialogInfo := '';
   repeat
      ReadLn(fich, foo);
      if foo[1] <> '-' then
         dialogInfo := dialogInfo + foo + #13 + #10;
   until foo[1] = '-';
   Readln(fich, newGroup);
   CloseFile(fich);

   DeleteFile(pathInfoFile);

   AssignFile(fich, 'C:\PATHINFO.INI');
   Rewrite(fich);
   longPath := programsFolder + newGroup + '                                                                 ';
   GetShortPathName(PChar(longPath),PChar(longPath),256);
   WriteLn(fich, PChar(longPath));
   longPath := desktopFolder + '                                                                             ';
   GetShortPathName(PChar(longPath),PChar(longPath),256);
   WriteLn(fich, PChar(longPath));
   CloseFile(fich);

   ShellExecute(GetDesktopWindow(), 'open', PChar(ExtractFilePath(Application.ExeName) + exeName), nil, PChar(ExtractFilePath(Application.ExeName)), SW_SHOWNORMAL);
   while not FileExists(pathInfoFile) do ;
   Sleep(1000);

   AssignFile(pathfich, pathInfoFile);
   Reset(pathfich);
   ReadLn(pathfich, pathInfo);
   userCancel := pathInfo = 'null';
   CloseFile(pathfich);

   if not userCancel then begin
      AssignFile(fich, scriptFile);
      Reset(fich);
      ReadLn(fich, foo);
      Readln(fich, pathInfoFile);
      ReadLn(fich, foo);
      Readln(fich, exeName);
      ReadLn(fich, foo);
      Readln(fich, dialogTitle);
      ReadLn(fich, foo);
      dialogInfo := '';
      repeat
         ReadLn(fich, foo);
         if foo[1] <> '-' then
            dialogInfo := dialogInfo + foo + #13 + #10;
      until foo[1] = '-';
      Readln(fich, newGroup);
      ReadLn(fich, foo);
{$I-}
      if not DirectoryExists(programsFolder + newGroup) then
         MkDir(programsFolder + newGroup);
{$I+}
      repeat
         ReadLn(fich, linkText);
         if linkText[1] <> '-' then begin
            ReadLn(fich, linkFile);
            CopyFile(PChar(pathInfo + linkFile), PChar(programsFolder + newGroup + '\' + linkText), false);
         end;
      until linkText[1] = '-';
      repeat
         ReadLn(fich, linkText);
         if linkText[1] <> '-' then begin
            ReadLn(fich, linkFile);
            CopyFile(PChar(pathInfo + linkFile), PChar(desktopFolder + '\' + linkText), false);
         end;
      until linkText[1] = '-';

      Close(fich);
      MessageBox(GetDesktopWindow(), PChar(dialogInfo), PChar(dialogTitle), MB_OK);
   end;
   DeleteFile(pathInfoFile);
end.

