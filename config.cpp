class CfgPatches
{
	class EditorUGTriggers
	{
		units[] = {""};
		weapons[] = {};
		requiredVersion = 1.0;
		requiredAddons[] = {"DZ_Data"};
	};
};
class CfgMods
{
	class EditorUGTriggers
	{
		dir = "EditorUGTriggers";
		name = "EditorUGTriggers";
		credits = "BobbyDale";
		author = "BobbyDale";
		authorID = "0";
		version = "1.0";
		extra = 0;
		type = "mod";
		inputs = "EditorUGTriggers/Scripts/Data/Inputs.xml";
		dependencies[] = {"Game","World","Mission"};
		class defs
		{
			class gameScriptModule
			{
				value = "";
				files[] = {"EditorUGTriggers/scripts/3_Game"};
			};
			class worldScriptModule
			{
				value = "";
				files[] = {"EditorUGTriggers/scripts/4_World"};
			};
			class missionScriptModule
			{
				value = "";
				files[] = {"EditorUGTriggers/scripts/5_Mission"};
			};
		};
	};
};
class CfgVehicles
{
	class HouseNoDestruct;	
	class UGTriggerObject: HouseNoDestruct
	{
		scope = 1;
		displayName = "Editor UG Trigger";
		model = "EditorUGTriggers\models\1x1.p3d";
	};
	class UGBreadcrumb: HouseNoDestruct
	{
		scope = 1;
		displayName = "Editor UG Breadcrumb";
		model = "EditorUGTriggers\models\breadcrumb.p3d";
	};
};