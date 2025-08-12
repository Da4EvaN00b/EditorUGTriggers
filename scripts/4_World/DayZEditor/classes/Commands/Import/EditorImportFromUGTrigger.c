class EditorImportFromUG: EditorImportCommandBase
{
	override typename GetFileType() 
	{
		return EditorUGTriggerFile;
	}
	
	override string GetName() 
	{
		return "Import from UG Triggers (*.json)";
	}
}
