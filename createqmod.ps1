Param(
    [Parameter(Mandatory=$false)]
    [String] $qmodName="",

    [Parameter(Mandatory=$false)]
    [Switch] $help
)

if ($help -eq $true) {
    Write-Output "`"createqmod`" - Creates a .qmod file with your compiled libraries and mod.json."
    Write-Output "`n-- Arguments --`n"

    Write-Output "-QmodName `t The file name of your qmod"

    exit
}

$mod = "./mod.json"

& $PSScriptRoot/validate-modjson.ps1
if ($LASTEXITCODE -ne 0) {
    exit $LASTEXITCODE
}
$modJson = Get-Content $mod -Raw | ConvertFrom-Json

if ($qmodName -eq "") {
    $qmodName = $modJson.name
}

$filelist = @($mod)

$cover = "./" + $modJson.coverImage
if ((-not ($cover -eq "./")) -and (Test-Path $cover)) {
    $filelist += ,$cover
}

foreach ($modFile in $modJson.modFiles) {
    $path = "./build/" + $modFile
    if (-not (Test-Path $path)) {
        $path = "./extern/libs/" + $modFile
    }
    if (-not (Test-Path $path)) {
        Write-Output "Error: could not find dependency: $path"
        exit 1
    }
    $filelist += $path
}

foreach ($lib in $modJson.libraryFiles) {
    $path = "./build/" + $lib
    if (-not (Test-Path $path)) {
        $path = "./extern/libs/" + $lib
    }
    if (-not (Test-Path $path)) {
        Write-Output "Error: could not find dependency: $path"
        exit 1
    }
    $filelist += $path
}

$destinationPath = "/sdcard/ModData/" + $modJson.packageId + "/Mods/" + $modJson.id

Get-ChildItem "./images/" | ForEach-Object {
    $name = $_.NameString
    $filelist += "./images/" + $name
    $modJson.fileCopies += (ConvertFrom-Json -InputObject "{
        `"name`": `"$name`",
        `"destination`": `"$destinationPath/$name`"
    }")
}

ConvertTo-Json -InputObject $modJson -Depth 32 | Set-Content $mod

$zip = $qmodName + ".zip"
$qmod = $qmodName + ".qmod"

Compress-Archive -Path $filelist -DestinationPath $zip -Update
Start-Sleep 1
Move-Item $zip $qmod -Force
