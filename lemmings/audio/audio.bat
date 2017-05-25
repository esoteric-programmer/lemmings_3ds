echo off
mode con: cols=113 lines=30
echo.
echo.
echo This is a windows batch file created by TarkinMX. This batch file uses VLC and 7-Zip to extract audio files
echo from PlayerSource_V29.zip and lem_wavs.zip for use with bayleef's Lemmings for 3DS. If VLC and/or 7-Zip aren't
echo present, then this batch file will use .Net 4.5 to download their portable versions. PlayerSource_V29.zip will
echo be required for music and sound files, lem_wavs.zip is only needed if you'd prefer those sound files. Therefore,
echo the requirements to run this batch file are PlayerSource_V29.zip and then .Net 4.5 framework -OR- VLC and 7-Zip.
echo If you do not meet these requirements then there will be errors.
echo.
timeout /t 30

if exist "PlayerSource_V29.zip" (
  echo.
  echo PlayerSource_V29.zip found
) else (
  cls
  echo.
  echo.
  echo PlayerSource_V29.zip not found. Please make sure PlayerSource_V29.zip is in this folder.
  echo.
  echo Press any key to quit.
  pause >nul
  exit
)
echo.
echo getting paths for VLC and 7-Zip

if exist "%ProgramFiles%\7-Zip\7z.exe" (
  set zippath="%ProgramFiles%\7-Zip"
  goto VLCsetup
) else if exist "%ProgramFiles(x86)%\7-Zip\7z.exe" (
  set zippath="%ProgramFiles(x86)%\7-Zip"
  goto VLCsetup
) else if exist "7za.exe" ( goto 7zset ) else if exist "7z.exe" ( goto 7zset )

echo.
echo No paths for 7-Zip were found, attempting to download 7-zip command line version using .Net 4.5
powershell.exe (new-object System.Net.WebClient).DownloadFile('http://www.7-zip.org/a/7za920.zip','.\7za920.zip')
echo.
if exist "7za920.zip" (
  echo attempting to extract 7-Zip
  )
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem';[IO.Compression.ZipFile]::ExtractToDirectory('7za920.zip', '.'); }"

:7zset
set zippath="7-Zip command line version"
if exist "7za.exe" (
  ren 7za.exe 7z.exe
  if exist 7za920.zip del 7za920.zip
  if exist 7-zip.chm del 7-zip.chm
  if exist license.txt del license.txt
  echo done
) else (
  cls
  echo.
  echo.
  echo No paths for 7-Zip were found and downloading the command line version was unsuccessful. Make sure there is
  echo a working "7za.exe" in this folder.
  echo.
  echo Press any key to quit.
  pause >nul
  exit
)

:VLCsetup
if exist "%ProgramFiles%\VideoLAN\VLC\vlc.exe" (
  set VLCpath=%ProgramFiles%\VideoLAN\VLC"
  goto main
) else if exist "%ProgramFiles(x86)%\VideoLAN\VLC\vlc.exe" (
  set VLCpath="%ProgramFiles(x86)%\VideoLAN\VLC"
  goto main
) else if exist "vlc.exe" ( goto VLCset )

echo.
echo No paths for VLC were found, attempting to download VLC portable version using .Net 4.5, please be patient as
echo this will take a while.
powershell.exe (new-object System.Net.WebClient).DownloadFile('http://get.videolan.org/vlc/2.2.4/win32/vlc-2.2.4-win32.zip','.\vlc-2.2.4-win32.zip')
echo.
echo attempting to extract VLC
set PATH=%PATH%;%zippath%
7z e vlc-2.2.4-win32.zip vlc-2.2.4\vlc.exe
7z e vlc-2.2.4-win32.zip vlc-2.2.4\libvlc.dll
7z e vlc-2.2.4-win32.zip vlc-2.2.4\libvlccore.dll
7z x vlc-2.2.4-win32.zip vlc-2.2.4\plugins
move vlc-2.2.4\plugins
rd vlc-2.2.4
del vlc-2.2.4-win32.zip

:VLCset
if exist "vlc.exe" (
  set VLCpath="VLC portable version"
) else (
  cls
  echo.
  echo.
  echo No paths for VLC were found and downloading the portable version was unsuccessful. Make sure there is a working
  echo "vlc-2.2.4-win32.zip" in this folder.
  echo.
  echo Press any key to quit.
  pause >nul
  exit
)

:main
echo.
echo %VLCpath% found
echo.
echo %zippath% found
echo.
echo setting paths for VLC and 7-Zip
set PATH=%PATH%;%VLCpath%;%zippath%

if exist SFX*.WAV ( del SFX*.WAV )

if exist "lem_wavs.zip" (
  echo.
  echo "lem_wavs.zip found
  echo.
  echo processing sound files
  7z e lem_wavs.zip changeop.wav
  ren changeop.wav SFX01.WAV
  7z e lem_wavs.zip door.wav
  ren door.wav SFX02.WAV
  7z e lem_wavs.zip letsgo.wav
  ren letsgo.wav SFX03.WAV
  7z e lem_wavs.zip mousepre.wav
  ren mousepre.wav SFX04.WAV
  7z e lem_wavs.zip ohno.wav
  ren ohno.wav SFX05.WAV
  7z e lem_wavs.zip electric.wav
  ren electric.wav SFX06.WAV
  7z e lem_wavs.zip thud.wav
  ren thud.wav SFX07.WAV
  7z e lem_wavs.zip splat.wav
  ren splat.wav SFX08.WAV
  7z e lem_wavs.zip chain.wav
  ren chain.wav SFX09.WAV
  7z e lem_wavs.zip chink.wav
  ren chink.wav SFX10.WAV
  7z e lem_wavs.zip explode.wav
  ren explode.wav SFX12.WAV
  7z e lem_wavs.zip fire.wav
  ren fire.wav SFX13.WAV
  7z e lem_wavs.zip tenton.wav
  ren tenton.wav SFX14.WAV
  7z e lem_wavs.zip thunk.wav
  ren thunk.wav SFX15.WAV
  7z e lem_wavs.zip yippee.wav
  ren yippee.wav SFX16.WAV
  7z e lem_wavs.zip glug.wav
  ren glug.wav SFX17.WAV
  7z e lem_wavs.zip ting.wav
  ren ting.wav SFX18.WAV
  7z e lem_wavs.zip die.wav
  ren die.wav SFX19.WAV
) else (
  echo lem_wavs.zip not found, sound files will be processed from PlayerSource_V29.zip
  echo.
  echo processing sound files
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.0.wav
  ren BasicFX.0.wav SFX01.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.1.wav
  ren BasicFX.1.wav SFX02.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.2.wav
  ren BasicFX.2.wav SFX03.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.3.wav
  ren BasicFX.3.wav SFX04.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.4.wav
  ren BasicFX.4.wav SFX05.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.3.wav
  ren noises.3.wav SFX06.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.7.wav
  ren noises.7.wav SFX07.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.5.wav
  ren BasicFX.5.wav SFX08.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.1.wav
  ren noises.1.wav SFX09.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.6.wav
  ren BasicFX.6.wav SFX10.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.9.wav
  ren BasicFX.9.wav SFX12.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.2.wav
  ren noises.2.wav SFX13.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.1.wav
  ren FullFX.1.wav SFX14.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.2.wav
  ren FullFX.2.wav SFX15.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.7.wav
  ren BasicFX.7.wav SFX16.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.4.wav
  ren noises.4.wav SFX17.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.8.wav
  ren BasicFX.8.wav SFX18.WAV
  7z e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.10.wav
  ren BasicFX.8.wav SFX19.WAV
)

echo.
echo processing music files
md ohno
7z e PlayerSource_V29.zip PlayerSourceTrad\Music\ohno\*.it
for %%a in (*.it) do vlc -I dummy -vvv %%a --sout=#transcode{acodec=s16l,channels=2,samplerate=44100}:std{access=file,mux=wav,dst=ohno\%%a} vlc://quit
cd ohno
ren track_01.it TUNE01.WAV
ren track_02.it TUNE02.WAV
ren track_03.it TUNE03.WAV
ren track_04.it TUNE04.WAV
ren track_05.it TUNE05.WAV
ren track_06.it TUNE06.WAV
cd..
attrib *.it -r
del *.it

md orig
7z e PlayerSource_V29.zip PlayerSourceTrad\Music\orig\*.it
for %%a in (*.it) do vlc -I dummy -vvv %%a --sout=#transcode{acodec=s16l,channels=2,samplerate=44100}:std{access=file,mux=wav,dst=orig\%%a} vlc://quit
%a} vlc://quit
cd orig
ren track_20.it TUNE01.WAV
ren track_18.it TUNE02.WAV
ren track_21.it TUNE03.WAV
ren track_01.it TUNE04.WAV
ren track_08.it TUNE05.WAV
ren track_02.it TUNE06.WAV
ren track_04.it TUNE07.WAV
ren track_10.it TUNE08.WAV
ren track_19.it TUNE09.WAV
ren track_17.it TUNE10.WAV
ren track_16.it TUNE11.WAV
ren track_13.it TUNE12.WAV
ren track_03.it TUNE13.WAV
ren track_06.it TUNE14.WAV
ren track_15.it TUNE15.WAV
ren track_07.it TUNE16.WAV
ren track_09.it TUNE17.WAV
ren track_11.it TUNE18.WAV
ren track_05.it TUNE19.WAV
ren track_12.it TUNE20.WAV
ren track_14.it TUNE21.WAV
cd..
attrib *.it -r
del *.it

md xmas
7z e PlayerSource_V29.zip PlayerSourceTrad\Music\h94\*.it
for %%a in (*.it) do vlc -I dummy -vvv %%a --sout=#transcode{acodec=s16l,channels=2,samplerate=44100}:std{access=file,mux=wav,dst=xmas\%%a} vlc://quit
cd xmas
ren track_01.it TUNE01.WAV
ren track_02.it TUNE02.WAV
ren track_03.it TUNE03.WAV
cd..
attrib *.it -r
del *.it

:choice
echo.
set /P c=" Delete all the .zip files and any other loose files from this folder? (Y/N) "
if /I "%c%" EQU "Y" (
  echo.
  set /P c=" Are you sure? (Y/N) "
  if /I "%c%" EQU "Y" goto :yes
  if /I "%c%" EQU "N" goto :no
)
if /I "%c%" EQU "N" goto :no
echo.
echo Invalid Option, must be (Y) or (N)
goto choice

:yes
echo.
echo OK, cleaning up zip files and loose files.
  if exist 7z.exe ( del 7z.exe )
  if exist vlc.exe ( del vlc.exe )
  if exist libvlc.dll ( del libvlc.dll )
  if exist libvlccore.dll ( del libvlccore.dll )
  if exist plugins ( rd plugins /s /q )

:no
echo.
echo Finished. Press any key to quit.
pause >nul
