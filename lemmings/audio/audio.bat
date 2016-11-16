echo off
cls
echo.
echo.
echo This batch file requires VLC for converting the music files and .NET 4.5 to download 7-Zip if it's not present. If you do not meet these requirements then there will be errors.
echo.
timeout /t 5

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
if exist "%ProgramFiles%\VideoLAN\VLC\vlc.exe" (
  set VLCpath=%ProgramFiles%\VideoLAN\VLC"
) else if exist "%ProgramFiles(x86)%\VideoLAN\VLC\vlc.exe" (
  set VLCpath="%ProgramFiles(x86)%\VideoLAN\VLC"
) else (
  cls
  echo.
  echo.
  echo No paths for VLC were found. Is VLC properly installed?
  echo.
  echo Press any key to quit.
  pause >nul
  exit
)

if exist "%ProgramFiles%\7-Zip\7z.exe" (
  set zippath="%ProgramFiles%\7-Zip"
  set unzip="7z.exe"
  goto main
) else if exist "%ProgramFiles(x86)%\7-Zip\7z.exe" (
  set zippath="%ProgramFiles(x86)%\7-Zip"
  set unzip="7z.exe"
  goto main
) else (
  goto download
)

:download
  echo.
  echo No paths for 7-Zip were found, attempting to download 7-zip command line version using .Net 4.5
  powershell.exe (new-object System.Net.WebClient).DownloadFile('http://www.7-zip.org/a/7za920.zip','.\7za920.zip')
  echo.
  echo attempting to extract 7-Zip
  powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem';[IO.Compression.ZipFile]::ExtractToDirectory('7za920.zip', '.'); }"
  if exist "7za.exe" (
  set zippath="7-Zip command line version"
  set unzip="7za.exe"
  ) else (
  cls
  echo.
  echo.
  echo No paths for 7-Zip were found and downloading the command line version was unsuccessful. Make sure there is a working "7za.exe" in this folder.
  echo.
  echo Press any key to quit.
  pause >nul
  exit
  )
  goto main

:main
echo.
echo %VLCpath% found
echo.
echo %zippath% found
echo.
echo 7-Zip executable was found as %unzip%
echo.
echo setting paths for VLC and 7-Zip
set PATH=%PATH%;%VLCpath%;%zippath%

if exist "lem_wavs.zip" (
  echo.
  echo lem_wavs.zip found
  echo.
  echo processing sound files
  %unzip% e lem_wavs.zip changeop.wav
  ren changeop.wav SFX01.WAV
  %unzip% e lem_wavs.zip door.wav
  ren door.wav SFX02.WAV
  %unzip% e lem_wavs.zip letsgo.wav
  ren letsgo.wav SFX03.WAV
  %unzip% e lem_wavs.zip mousepre.wav
  ren mousepre.wav SFX04.WAV
  %unzip% e lem_wavs.zip ohno.wav
  ren ohno.wav SFX05.WAV
  %unzip% e lem_wavs.zip electric.wav
  ren electric.wav SFX06.WAV
  %unzip% e lem_wavs.zip thud.wav
  ren thud.wav SFX07.WAV
  %unzip% e lem_wavs.zip splat.wav
  ren splat.wav SFX08.WAV
  %unzip% e lem_wavs.zip chain.wav
  ren chain.wav SFX09.WAV
  %unzip% e lem_wavs.zip chink.wav
  ren chink.wav SFX10.WAV
  %unzip% e lem_wavs.zip explode.wav
  ren explode.wav SFX12.WAV
  %unzip% e lem_wavs.zip fire.wav
  ren fire.wav SFX13.WAV
  %unzip% e lem_wavs.zip tenton.wav
  ren tenton.wav SFX14.WAV
  %unzip% e lem_wavs.zip thunk.wav
  ren thunk.wav SFX15.WAV
  %unzip% e lem_wavs.zip yippee.wav
  ren yippee.wav SFX16.WAV
  %unzip% e lem_wavs.zip glug.wav
  ren glug.wav SFX17.WAV
  %unzip% e lem_wavs.zip ting.wav
  ren ting.wav SFX18.WAV
) else (
  echo lem_wavs.zip not found, sound files will be processed from PlayerSource_V29.zip
  echo.
  echo processing sound files
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.0.wav
  ren BasicFX.0.wav SFX01.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.1.wav
  ren BasicFX.1.wav SFX02.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.2.wav
  ren BasicFX.2.wav SFX03.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.3.wav
  ren BasicFX.3.wav SFX04.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.4.wav
  ren BasicFX.4.wav SFX05.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.3.wav
  ren noises.3.wav SFX06.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.7.wav
  ren noises.7.wav SFX07.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.5.wav
  ren BasicFX.5.wav SFX08.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.1.wav
  ren noises.1.wav SFX09.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.6.wav
  ren BasicFX.6.wav SFX10.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.9.wav
  ren BasicFX.9.wav SFX12.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.2.wav
  ren noises.2.wav SFX13.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.1.wav
  ren FullFX.1.wav SFX14.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.2.wav
  ren FullFX.2.wav SFX15.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.7.wav
  ren BasicFX.7.wav SFX16.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.4.wav
  ren noises.4.wav SFX17.WAV
  %unzip% e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.8.wav
  ren BasicFX.8.wav SFX18.WAV
)

echo.
echo processing music files
md ohno
%unzip% e PlayerSource_V29.zip PlayerSourceTrad\Music\ohno\*.it
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
%unzip% e PlayerSource_V29.zip PlayerSourceTrad\Music\orig\*.it
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
%unzip% e PlayerSource_V29.zip PlayerSourceTrad\Music\h94\*.it
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
set /P c=Delete the zip files and any other loose files from this folder? (Y/N)
if /I "%c%" EQU "Y"
  goto :yes
if /I "%c%" EQU "N"
  goto :no
echo.
echo Invalid Option, must be (Y) or (N)
goto choice

:yes
  echo.
  Cleaning up loose files
  del *.zip
  del 7za.exe
  del 7-zip.chm
  del license.txt
  goto no

:no
  echo.
  echo Finished. Press any key to quit.
  pause >nul
  exit
