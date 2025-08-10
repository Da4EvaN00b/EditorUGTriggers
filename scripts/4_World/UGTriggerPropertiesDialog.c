modded class EditorObjectPropertiesDialog : EditorDialogBase
{
	// === UG fields bound to the UI ===
	vector UG_SizeVec;
	float  UG_EyeAccommodation = 1.0; // [0..1]
	float  UG_Interpolation    = 1.0; // [0..1]
	int    UG_Type             = 0;   // 0=Outer, 1=Inner, 2=Transitional
	int    UG_LastType = -1;
	float BC_EyeAccommodation = 1.0; // 0..1
	int   BC_UseRaycast = 0;         // 0/1
	float BC_Radius = -1.0;          // -1 = default

	// ---------- SINGLE SELECTION ----------
	override void SetEditorObject(EditorObject editor_object)
	{
		// Build all default groups first
		super.SetEditorObject(editor_object);

    	UGTriggerObject ug = UGTriggerObject.Cast(editor_object.GetWorldObject());
    	if (ug)
		{
    		UG_SizeVec          = ug.GetSize();
    		UG_EyeAccommodation = ug.GetEyeAccommodation();
    		UG_Interpolation    = ug.GetInterpolation();
    		UG_Type             = ug.GetUGType();     // <-- read the actual type
    		UG_LastType         = UG_Type;            // seed so defaults apply only on real type change

    		GroupPrefab ug_group = new GroupPrefab("Underground Trigger", this, string.Empty);
    		ug_group.Insert(new VectorPrefab("Size (X,Y,Z)", this, "UG_SizeVec"));

    		DropdownListPrefab<int> type_dropdown = new DropdownListPrefab<int>("Type", this, "UG_Type");
    		type_dropdown["Outer"]        = 0;
    		type_dropdown["Inner"]        = 1;
    		type_dropdown["Transitional"] = 2;
    		ug_group.Insert(type_dropdown);

    		ug_group.Insert(new EditBoxNumberPrefab("Eye Accommodation", this, "UG_EyeAccommodation", 0.01, 0.0, 1.0));
    		ug_group.Insert(new EditBoxNumberPrefab("Interpolation Speed", this, "UG_Interpolation", 0.01, 0.0, 1.0));
    		AddContent(ug_group);
		}
		// Breadcrumb UI (when a UGBreadcrumb is selected)
		UGBreadcrumb bc_obj = UGBreadcrumb.Cast(editor_object.GetWorldObject());
			if (!ug)
		{
			if (bc_obj)
			{
				BC_EyeAccommodation = bc_obj.GetEyeAccommodation();
				BC_UseRaycast      = bc_obj.GetUseRaycast();
				BC_Radius          = bc_obj.GetRadius();

				GroupPrefab bc_group = new GroupPrefab("Breadcrumb Properties", this, string.Empty);
				bc_group.Insert(new EditBoxNumberPrefab("Eye Accommodation", this, "BC_EyeAccommodation", 0.01, 0.0, 1.0));

				DropdownListPrefab<int> bc_raycast = new DropdownListPrefab<int>("Use Raycast", this, "BC_UseRaycast");
				bc_raycast["No"] = 0; bc_raycast["Yes"] = 1; 
				bc_group.Insert(bc_raycast);
				bc_group.Insert(new EditBoxNumberPrefab("Radius", this, "BC_Radius", 0.1, -1.0, 10000.0));

				AddContent(bc_group);
			}
		}
	}
	// ---------- MULTI SELECTION ----------
	override void SetMultipleEditorObjects(array<EditorObject> editor_objects)
	{
	    super.SetMultipleEditorObjects(editor_objects);

	    bool seeded = false;
	    vector firstSize;
	    float firstAcc = 1.0;
	    float firstInterp = 1.0;
	    int   firstType = 0;

	    foreach (EditorObject eo : editor_objects)
	    {
	        UGTriggerObject ug = UGTriggerObject.Cast(eo.GetWorldObject());
	        if (!ug) continue;

	        if (!seeded)
	        {
	            firstSize  = ug.GetSize();
	            firstAcc   = ug.GetEyeAccommodation();
	            firstInterp= ug.GetInterpolation();
	            firstType  = ug.GetUGType();   // <-- read actual type
	            seeded = true;
	        }
	    }

	    if (!seeded) return;

	    UG_SizeVec          = firstSize;
	    UG_EyeAccommodation = firstAcc;
	    UG_Interpolation    = firstInterp;
	    UG_Type             = firstType;       // <-- no mapping from EA
	    UG_LastType         = UG_Type;

	    GroupPrefab ug_group_multi = new GroupPrefab("Underground Trigger (Selection)", this, string.Empty);
	    ug_group_multi.Insert(new VectorPrefab("Size (X,Y,Z)", this, "UG_SizeVec"));

	    DropdownListPrefab<int> type_dropdown = new DropdownListPrefab<int>("Type", this, "UG_Type");
	    type_dropdown["Outer"]        = 0;
	    type_dropdown["Inner"]        = 1;
	    type_dropdown["Transitional"] = 2;
	    ug_group_multi.Insert(type_dropdown);

	    ug_group_multi.Insert(new EditBoxNumberPrefab("Eye Accommodation", this, "UG_EyeAccommodation", 0.01, 0.0, 1.0));
	    ug_group_multi.Insert(new EditBoxNumberPrefab("Interpolation Speed", this, "UG_Interpolation", 0.01, 0.0, 1.0));
	    ug_group_multi.Insert(new ButtonPrefab("Apply to Selection", this, "UG_ApplyToSelection"));
	    AddContent(ug_group_multi);
	}

	// ---------- APPLY / CHANGE HANDLERS ----------
	protected vector UG_ClampSizeVec(vector v)
	{
		if (v[0] < 1.0) v[0] = 1.0;
		if (v[1] < 1.0) v[1] = 1.0;
		if (v[2] < 1.0) v[2] = 1.0;
		return v;
	}

	override void PropertyChanged(string property_name)
	{
	super.PropertyChanged(property_name);

    if (property_name == "BC_EyeAccommodation" || property_name == "BC_UseRaycast" || property_name == "BC_Radius")
    {
        if (!m_EditorObject) return;
        UGBreadcrumb bc = UGBreadcrumb.Cast(m_EditorObject.GetWorldObject());
        if (!bc) return;

        if (property_name == "BC_EyeAccommodation")
        {
            if (BC_EyeAccommodation < 0.0) BC_EyeAccommodation = 0.0;
            if (BC_EyeAccommodation > 1.0) BC_EyeAccommodation = 1.0;
            bc.SetEyeAccommodation(BC_EyeAccommodation);
            return;
        }
        if (property_name == "BC_UseRaycast")
        {
            bc.SetUseRaycast(BC_UseRaycast);
            return;
        }
        if (property_name == "BC_Radius")
        {
            bc.SetRadius(BC_Radius);
            return;
        }
	}
	if (!m_EditorObject) return;
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;

		UndergroundTrigger trig = ug.GetLinkedTrigger();

		if (property_name == "UG_SizeVec")
		{
			UG_SizeVec = UG_ClampSizeVec(UG_SizeVec);
			ug.SetSize(UG_SizeVec); // applies transform + trigger extents
			return;
		}

		if (property_name == "UG_Type")
		{

    		if (UG_LastType != UG_Type) 
			{
    		    if (UG_Type == 0) UG_EyeAccommodation = 1.00; else UG_EyeAccommodation = 0.00;
    		    UG_LastType = UG_Type;
    		}
    		ug.SetUGType(UG_Type);
    		ug.SetEyeAccommodation(Math.Round(Math.Clamp(UG_EyeAccommodation,0.0,1.0)*100.0)/100.0);
    		return;

		}

		if (property_name == "UG_EyeAccommodation")
		{
        	UG_EyeAccommodation = Math.Clamp(Math.Round(UG_EyeAccommodation*100.0)/100.0, 0.0, 1.0);
        	ug.SetEyeAccommodation(UG_EyeAccommodation);
        	return;
		}

		if (property_name == "UG_Interpolation")
		{
        	UG_Interpolation = Math.Clamp(UG_Interpolation, 0.0, 1.0);
        	ug.SetInterpolation(UG_Interpolation);
        	return;
		}
	}

	void UG_ApplyToSelection()
	{
		UG_SizeVec          = UG_ClampSizeVec(UG_SizeVec);
		UG_EyeAccommodation = Math.Clamp(UG_EyeAccommodation, 0.0, 1.0);
		UG_Interpolation    = Math.Clamp(UG_Interpolation, 0.0, 1.0);

		foreach (EditorObject eo : m_EditorObjects)
		{
			UGTriggerObject ug = UGTriggerObject.Cast(eo.GetWorldObject());
			if (!ug) continue;

			ug.SetSize(UG_SizeVec);

			UndergroundTrigger trig = ug.GetLinkedTrigger();
			if (trig)
			{
				trig.m_Accommodation      = UG_EyeAccommodation;
				trig.m_InterpolationSpeed = UG_Interpolation;
				// NOTE: Type is not exported; it's just a preset for EyeAccommodation
			}
		}
	}

	void OnCollectBreadcrumbs()
	{
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;
		ug.CollectBreadcrumbs();
	}

	void OnUGTypeChanged()
	{
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;
		if (UG_Type == 0) UG_EyeAccommodation = 1.00;
		else if (UG_Type == 1) UG_EyeAccommodation = 0.00;
		else UG_EyeAccommodation = 0.00;
		ug.SetUGType(UG_Type);
		ug.SetEyeAccommodation(UG_EyeAccommodation);
	}

	void OnUGEyeAccommodationChanged()
	{
		UG_EyeAccommodation = UG_Round2(UG_EyeAccommodation);
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;
		ug.SetEyeAccommodation(UG_EyeAccommodation);
	}

	void OnUGInterpolationChanged()
	{
		UG_Interpolation = Math.Clamp(UG_Interpolation, 0.0, 1.0);
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;
		ug.SetInterpolation(UG_Interpolation);
	}

	void OnUGSizeChanged()
	{
		UGTriggerObject ug = UGTriggerObject.Cast(m_EditorObject.GetWorldObject());
		if (!ug) return;
		ug.SetSize(UG_SizeVec);
	}
}