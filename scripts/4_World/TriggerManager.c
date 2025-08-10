static const float UG_STEP = 1;

modded class Editor
{
    override void ProcessInput(float dt, Input input)
    {
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

            //Just for testing, Press Num Pad 5 to add breadcrumbs. Should be worked into the update loop later. 
            if (input_api.GetInputByName("UGTRIG_AddBreadcrumbs").LocalPress())  UG_AddBreadCrumbs({triggerobj});

        }

        // --- Post-process fix so keyboard/mouse move keeps UGTriggerObject scale ---
        UG_ReapplyScaleForSelection();
    }

    void UG_ReapplyScaleForSelection()
    {
      //  Print("[UGTriggerObject] Reapplying scale for selection...");
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

    static void UG_AddBreadCrumbs(array<UGTriggerObject> ugs)
    {
        Print("[UGTriggerObject] UG_AddBreadCrumbs called for " + ugs.Count() + " UGTriggerObjects");
        if (ugs.Count() == 0) return;
        if (!GetGame().IsServer()) return;

        foreach (UGTriggerObject ug : ugs) {
            ug.GetBreadcrumbs(); 
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

            float acc    = Math.Clamp(triggerobj.GetEyeAccommodation(), 0.0, 1.0);
            float interp = Math.Clamp(triggerobj.GetInterpolation(),    0.0, 1.0);
            
            UndergroundTrigger trig = triggerobj.GetLinkedTrigger();

            // Build export row
            UGTriggersExport ex = new UGTriggersExport(pos, orient, size, acc, interp);

            // Breadcrumbs ONLY for Transitional and when there are at least 2
            if (triggerobj.GetUGType() == 2 && trig && trig.m_Data && trig.m_Data.Breadcrumbs && trig.m_Data.Breadcrumbs.Count() >= 2)
            {
                ex.Breadcrumbs = new array<ref UGBreadcrumbExport>();
                foreach (JsonUndergroundAreaBreadcrumb b : trig.m_Data.Breadcrumbs)
                {
                    UGBreadcrumbExport eb = new UGBreadcrumbExport();
                    eb.Position = new array<float>();
                    eb.Position.Insert(b.Position.Get(0));
                    eb.Position.Insert(b.Position.Get(1));
                    eb.Position.Insert(b.Position.Get(2));

                    eb.EyeAccommodation = b.EyeAccommodation;
                    eb.UseRaycast       = b.UseRaycast;
                    eb.Radius           = b.Radius;
                    ex.Breadcrumbs.Insert(eb);
                }
            }

            // IMPORTANT: insert INSIDE the loop
            exportData.Triggers.Insert(ex);
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
	ref array<float> Position;        // [x,y,z]
	ref array<float> Orientation;     // [yaw,pitch,roll]
	ref array<float> Size;            // [x,y,z]
	float            EyeAccommodation;
	float            InterpolationSpeed;
	ref array<ref UGBreadcrumbExport> Breadcrumbs;  // set only for Transitional



	void UGTriggersExport(vector pos, vector orient, vector size, float acc, float interp)
	{
		// Enforce min size rule to match editor UI
		if (size[0] < 1) size[0] = 1;
		if (size[1] < 1) size[1] = 1;
		if (size[2] < 1) size[2] = 1;

		Position = new array<float>();
		Position.Insert(pos[0]);    Position.Insert(pos[1]);    Position.Insert(pos[2]);

		Orientation = new array<float>();
		Orientation.Insert(orient[0]); Orientation.Insert(orient[1]); Orientation.Insert(orient[2]);

		Size = new array<float>();
		Size.Insert(size[0]); Size.Insert(size[1]); Size.Insert(size[2]);

		EyeAccommodation   = Math.Clamp(acc, 0.0, 1.0);
		InterpolationSpeed = Math.Clamp(interp, 0.0, 1.0);
	}
}

class UGBreadcrumbExport
{
	ref array<float> Position;  
	float EyeAccommodation;      
	int   UseRaycast;            
	float Radius;                
}
class UGTriggersExportRoot
{
    ref array<ref UGTriggersExport> Triggers = new array<ref UGTriggersExport>();
}

static JsonUndergroundAreaTriggerData BuildJsonFromUG(UGTriggerObject ug)
{
    if (!ug) return null;

    UndergroundTrigger t = ug.GetLinkedTrigger();

    vector pos    = ug.GetPosition();
    vector orient = ug.GetOrientation();
    vector size   = ug.GetSize();

    float acc = 1.0;
    float interp = 1.0;
    if (t) {
        acc    = t.m_Accommodation;
        interp = t.m_InterpolationSpeed;
    }

    JsonUndergroundAreaTriggerData d = new JsonUndergroundAreaTriggerData();

    // Position
    d.Position = new array<float>();
    d.Position.Insert(pos[0]);
    d.Position.Insert(pos[1]);
    d.Position.Insert(pos[2]);

    // Orientation
    d.Orientation = new array<float>();
    d.Orientation.Insert(orient[0]);
    d.Orientation.Insert(orient[1]);
    d.Orientation.Insert(orient[2]);

    // Size (this class expects Size, not min/max extents)
    d.Size = new array<float>();
    d.Size.Insert(size[0]);
    d.Size.Insert(size[1]);
    d.Size.Insert(size[2]);

    // Brightness blend controls
    d.EyeAccommodation = acc;        
    d.InterpolationSpeed = interp;   

    return d;
}