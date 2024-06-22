$mod = "./mod.json"
$modTemplate = Get-Item "./mod.template.json"
$qpmShared = Get-Item "./qpm.shared.json"

if (-not (Test-Path -Path $mod) -or $modTemplate.LastWriteTime -gt (Get-Item $mod).LastWriteTime -or $modTemplate.LastWriteTime -gt $qpmShared.LastWriteTime) {
    if (Test-Path -Path ".\mod.template.json") {
        & qpm qmod manifest
        if ($LASTEXITCODE -ne 0) {
            exit $LASTEXITCODE
        }

        $modJson = Get-Content $mod -Raw | ConvertFrom-Json
        $destinationPath = "/sdcard/ModData/" + $modJson.packageId + "/Mods/" + $modJson.id

        Get-ChildItem "./images/" | ForEach-Object {
            $name = $_.NameString
            $modJson.fileCopies += (ConvertFrom-Json -InputObject "{
                `"name`": `"$name`",
                `"destination`": `"$destinationPath/$name`"
            }")
        }

        ConvertTo-Json -InputObject $modJson -Depth 32 | Set-Content $mod
    }
    else {
        Write-Output "Error: mod.json and mod.template.json were not present"
        exit 1
    }
}

$psVersion = $PSVersionTable.PSVersion.Major
if ($psVersion -ge 6) {
    $schemaUrl = "https://raw.githubusercontent.com/Lauriethefish/QuestPatcher.QMod/main/QuestPatcher.QMod/Resources/qmod.schema.json"
    Invoke-WebRequest $schemaUrl -OutFile ./mod.schema.json

    $schema = "./mod.schema.json"
    $modJsonRaw = Get-Content $mod -Raw
    $modSchemaRaw = Get-Content $schema -Raw

    Remove-Item $schema

    Write-Output "Validating mod.json..."
    if (-not ($modJsonRaw | Test-Json -Schema $modSchemaRaw)) {
        Write-Output "Error: mod.json is not valid"
        exit 1
    }
}
else {
    Write-Output "Could not validate mod.json with schema: powershell version was too low (< 6)"
}
exit
