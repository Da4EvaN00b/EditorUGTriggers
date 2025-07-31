static const float UG_STEP = 1;

modded class Editor
{
    override void ProcessInput(float dt, Input input)
    {
        super.ProcessInput(dt, input);

        UAInputAPI input_api = GetUApi();
        if (!input_api) return;

        if (input_api.GetInputByName("UGTRIG_Export").LocalPress())
        {
            ExportUGTriggersToJSON();
        }

        foreach (int id, EditorObjectData obj_data : m_SessionCache)
        {
            EditorObject editor_object = GetPlacedObjectById(id);
            if (!editor_object || !editor_object.IsSelected())
                continue;

            UGTriggerObject triggerobj = UGTriggerObject.Cast(obj_data.WorldObject);
            if (!triggerobj)
                continue;

            if (input_api.GetInputByName("UGTRIG_IncLength").LocalPress())  triggerobj.TrigSize(UG_STEP, 0, 0);
            if (input_api.GetInputByName("UGTRIG_DecLength").LocalPress())  triggerobj.TrigSize(-UG_STEP, 0, 0);
            if (input_api.GetInputByName("UGTRIG_IncWidth").LocalPress())   triggerobj.TrigSize(0, 0, UG_STEP);
            if (input_api.GetInputByName("UGTRIG_DecWidth").LocalPress())   triggerobj.TrigSize(0, 0, -UG_STEP);
            if (input_api.GetInputByName("UGTRIG_IncHeight").LocalPress())  triggerobj.TrigSize(0, UG_STEP, 0);
            if (input_api.GetInputByName("UGTRIG_DecHeight").LocalPress())  triggerobj.TrigSize(0, -UG_STEP, 0);
        }
    }

    void ExportUGTriggersToJSON()
    {
        Print("[UGTriggers] Begin Export");

        ref UGTriggersExportRoot exportData = new UGTriggersExportRoot();

        foreach (int id, EditorObjectData obj_data : m_SessionCache)
        {
            UGTriggerObject triggerobj = UGTriggerObject.Cast(obj_data.WorldObject);
            if (!triggerobj)
                continue;

            vector pos = obj_data.Position;
            vector orient = obj_data.Orientation;
            vector size = triggerobj.GetSize();

            exportData.Triggers.Insert(new UGTriggersExport(pos, orient, size));
        }
		int year, month, day, hour, minute, second;
		GetHourMinuteSecondUTC(hour, minute, second);
		GetYearMonthDayUTC(year, month, day);

		string timestamp = string.Format("_%1-%2-%3_%4-%5-%6", year, month, day, hour, minute, second);
        string path = "$saves:Editor\\TriggerExport" + timestamp + ".json";

        JsonFileLoader<UGTriggersExportRoot>.JsonSaveFile(path, exportData);
        Print("[UGTriggers] Exported " + exportData.Triggers.Count() + " triggers to: " + path);
    }
}

class UGTriggersExport
{
    ref array<float> Position;
    ref array<float> Orientation;
    ref array<float> Size;
    int EyeAccommodation = 1;
    ref array<ref string> Breadcrumbs = {};
    int InterpolationSpeed = 1;

    void UGTriggersExport(vector pos, vector orient, vector size)
    {
        Position = { pos[0], pos[1], pos[2] };
        Orientation = { orient[0], orient[1], orient[2] };
        Size = { size[0], size[1], size[2] };
    }
}

class UGTriggersExportRoot
{
    ref array<ref UGTriggersExport> Triggers = new array<ref UGTriggersExport>();
}