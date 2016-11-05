echo off
cls
echo.
echo.
echo This batch file requires .NET 4.5 to download files and VLC for converting the music files, if you do not meet these two requirements there will be errors.
echo.
timeout /t 5

echo.
echo setting path for VLC
if exist "%ProgramFiles%\VideoLAN\VLC\vlc.exe" (
    goto setpath
) else if exist "%ProgramFiles(x86)%\VideoLAN\VLC\vlc.exe" (
    goto setpath86
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

:setpath86
    echo.
    echo "%ProgramFiles(x86)%\VideoLAN\VLC" found
    set PATH=%PATH%;"%ProgramFiles(x86)%\VideoLAN\VLC"
    goto main

:setpath
    echo.
    echo "%ProgramFiles%\VideoLAN\VLC" found
    set PATH=%PATH%;"%ProgramFiles%\VideoLAN\VLC"
    goto main

:main
echo.
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
echo downloading 7-zip command line version
powershell.exe (new-object System.Net.WebClient).DownloadFile('http://www.7-zip.org/a/7za920.zip','.\7za920.zip')

echo.
echo extracting 7-zip
powershell.exe -nologo -noprofile -command "& { Add-Type -A 'System.IO.Compression.FileSystem'; [IO.Compression.ZipFile]::ExtractToDirectory('7za920.zip', '.'); }"

echo.
if exist "lem_wavs.zip" (
    echo.
    echo "lem_wavs.zip found
    echo.
    echo processing sound files
    7za.exe e lem_wavs.zip changeop.wav
    ren changeop.wav SFX01.WAV
    7za.exe e lem_wavs.zip door.wav
    ren door.wav SFX02.WAV
    7za.exe e lem_wavs.zip letsgo.wav
    ren letsgo.wav SFX03.WAV
    7za.exe e lem_wavs.zip mousepre.wav
    ren mousepre.wav SFX04.WAV
    7za.exe e lem_wavs.zip ohno.wav
    ren ohno.wav SFX05.WAV
    7za.exe e lem_wavs.zip electric.wav
    ren electric.wav SFX06.WAV
    7za.exe e lem_wavs.zip thud.wav
    ren thud.wav SFX07.WAV
    7za.exe e lem_wavs.zip splat.wav
    ren splat.wav SFX08.WAV
    7za.exe e lem_wavs.zip chain.wav
    ren chain.wav SFX09.WAV
    7za.exe e lem_wavs.zip chink.wav
    ren chink.wav SFX10.WAV
    7za.exe e lem_wavs.zip explode.wav
    ren explode.wav SFX12.WAV
    7za.exe e lem_wavs.zip fire.wav
    ren fire.wav SFX13.WAV
    7za.exe e lem_wavs.zip tenton.wav
    ren tenton.wav SFX14.WAV
    7za.exe e lem_wavs.zip thunk.wav
    ren thunk.wav SFX15.WAV
    7za.exe e lem_wavs.zip yippee.wav
    ren yippee.wav SFX16.WAV
    7za.exe e lem_wavs.zip glug.wav
    ren glug.wav SFX17.WAV
    7za.exe e lem_wavs.zip ting.wav
    ren ting.wav SFX18.WAV
) else (
    echo lem_wavs.zip not found, sound files will be processed from PlayerSource_V29.zip
    echo.
    echo processing sound files
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.0.wav
    ren BasicFX.0.wav SFX01.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.1.wav
    ren BasicFX.1.wav SFX02.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.2.wav
    ren BasicFX.2.wav SFX03.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.3.wav
    ren BasicFX.3.wav SFX04.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.4.wav
    ren BasicFX.4.wav SFX05.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.3.wav
    ren noises.3.wav SFX06.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.7.wav
    ren noises.7.wav SFX07.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.5.wav
    ren BasicFX.5.wav SFX08.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.1.wav
    ren noises.1.wav SFX09.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.6.wav
    ren BasicFX.6.wav SFX10.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.9.wav
    ren BasicFX.9.wav SFX12.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.2.wav
    ren noises.2.wav SFX13.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.1.wav
    ren FullFX.1.wav SFX14.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\FullFX.2.wav
    ren FullFX.2.wav SFX15.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.7.wav
    ren BasicFX.7.wav SFX16.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\noises.4.wav
    ren noises.4.wav SFX17.WAV
    7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Sounds\amiga\BasicFX.8.wav
    ren BasicFX.8.wav SFX18.WAV
)


echo.
echo processing music files
md ohno
7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Music\ohno\*.it
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
7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Music\orig\*.it
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
7za.exe e PlayerSource_V29.zip PlayerSourceTrad\Music\h94\*.it
for %%a in (*.it) do vlc -I dummy -vvv %%a --sout=#transcode{acodec=s16l,channels=2,samplerate=44100}:std{access=file,mux=wav,dst=xmas\%%a} vlc://quit
cd xmas
ren track_01.it TUNE01.WAV
ren track_02.it TUNE02.WAV
ren track_03.it TUNE03.WAV
cd..
attrib *.it -r
del *.it

echo.
echo Cleaning up loose files
del 7za*.*
del 7-zip.chm
del license.txt
echo Finished. Press any key to quit.
pause >nul
