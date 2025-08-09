static const float UG_STEP = 1;

modded class Editor
{
    override void ProcessInput(float dt, Input input)
    {
        // Let the editor do all its normal logic first
        super.ProcessInput(dt, input);

        // --- Your UGTRIG custom keybinds ---
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

        // --- Post-process fix so keyboard/mouse move keeps UGTriggerObject scale ---
        UG_ReapplyScaleForSelection();
    }

    void UG_ReapplyScaleForSelection()
    {
        EditorObjectMap selected = GetSelectedObjects();
        if (!selected || selected.Count() == 0) return;

        foreach (int id, EditorObject eo : selected)
        {
            Object w = eo.GetWorldObject();
            UGTriggerObject ug = UGTriggerObject.Cast(w);
            if (!ug) continue;

            // Read the transform just set by the editor
            vector m[4];
            w.GetTransform(m);

            // Preserve axis directions but restore lengths from UGTriggerObject size
            vector ax0 = m[0].Normalized();
            vector ax1 = m[1].Normalized();
            vector ax2 = m[2].Normalized();

            vector s = ug.GetSize();
            m[0] = ax0 * s[0];
            m[1] = ax1 * s[1];
            m[2] = ax2 * s[2];

            w.SetTransform(m);
            w.Update();
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

			vector pos    = obj_data.Position;
			vector orient = obj_data.Orientation;
			vector size   = triggerobj.GetSize();

			// Read current trigger values (floats)
			float acc    = 1.0;
			float interp = 1.0;

			UndergroundTrigger trig = triggerobj.GetLinkedTrigger();
			if (trig)
			{
				acc    = Math.Clamp(trig.m_Accommodation, 0.0, 1.0);
				interp = Math.Clamp(trig.m_InterpolationSpeed, 0.0, 1.0);
			}

			exportData.Triggers.Insert(new UGTriggersExport(pos, orient, size, acc, interp));
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

// JSON payload classes
class UGTriggersExport
{
	ref array<float> Position;
	ref array<float> Orientation;
	ref array<float> Size;

	float EyeAccommodation;            // 0..1
	ref array<ref string> Breadcrumbs = {};
	float InterpolationSpeed;          // 0..1

	void UGTriggersExport(vector pos, vector orient, vector size, float acc, float interp)
	{
		// Enforce min size rule to match editor UI
		if (size[0] < 1) size[0] = 1;
		if (size[1] < 1) size[1] = 1;
		if (size[2] < 1) size[2] = 1;

		Position    = { pos[0], pos[1], pos[2] };
		Orientation = { orient[0], orient[1], orient[2] };
		Size        = { size[0], size[1], size[2] };

		EyeAccommodation  = Math.Clamp(acc, 0.0, 1.0);
		InterpolationSpeed = Math.Clamp(interp, 0.0, 1.0);
	}
}

class UGTriggersExportRoot
{
	ref array<ref UGTriggersExport> Triggers = new array<ref UGTriggersExport>();
}