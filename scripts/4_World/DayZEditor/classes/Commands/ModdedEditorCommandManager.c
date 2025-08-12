//Add to Export File Menu in Edior
modded class EditorCommandManager
{
	override void Init()
	{
        RegisterCommand(ExportUGTriggersToJSON);
        RegisterCommand(EditorImportFromUG);
        super.Init();    
    }
}