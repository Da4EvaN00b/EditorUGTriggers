class ExportUGTriggersToJSON: EditorExportCommandBase
{
	override typename GetFileType() 
	{
		return EditorUGTriggerFile;
	}
	
	override string GetName() 
	{
		return "Export to UG Triggers (*.json)";
	}
}
// - Adding Export option to Editor