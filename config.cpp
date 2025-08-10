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
		name = "EditorUGTriggers";
		version = "1.0";
		dir = "EditorUGTriggers";		
		author = "bauvdel & JinieJ";
		credits = "InclementDab";
		inputs = "EditorUGTriggers/Scripts/Data/Inputs.xml";
		type = "mod";
		dependencies[] = {"World"};
		class defs
		{
			class worldScriptModule
			{
				value = "";
				files[] = {"EditorUGTriggers/scripts/4_World"};
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