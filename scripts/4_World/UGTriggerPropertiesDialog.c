modded class EditorObjectPropertiesDialog : EditorDialogBase
{
	// === UG fields bound to the UI ===
	protected vector UG_SizeVec;
	protected float  UG_EyeAccommodation = 1.0; // [0..1]
	protected float  UG_Interpolation    = 1.0; // [0..1]
	protected int    UG_Type             = 0;   // 0=Outer, 1=Inner, 2=Transitional

	// ---------- SINGLE SELECTION ----------
	override void SetEditorObject(EditorObject editor_object)
	{
		// Build all default groups first
		super.SetEditorObject(editor_object);

		UGTriggerObject ug = UGTriggerObject.Cast(editor_object.GetWorldObject());
		if (!ug) return;

		// Seed from UG object + its trigger
		UG_SizeVec = ug.GetSize();

		UndergroundTrigger trig = ug.GetLinkedTrigger();
		if (trig)
		{
			UG_EyeAccommodation = Math.Clamp(trig.m_Accommodation, 0.0, 1.0);
			UG_Interpolation    = Math.Clamp(trig.m_InterpolationSpeed, 0.0, 1.0);

			// best-guess cosmetic default for Type from EA (no ternary)
			if (UG_EyeAccommodation >= 0.5)
				UG_Type = 0; // Outer
			else
				UG_Type = 1; // Inner (you can set a real rule for Transitional if you want)
		}

		// UG group: VectorPrefab lays out X/Y/Z on one line
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

	// ---------- MULTI SELECTION ----------
	override void SetMultipleEditorObjects(array<EditorObject> editor_objects)
	{
		// Build stock multi-select groups
		super.SetMultipleEditorObjects(editor_objects);

		// Detect any UG; seed from the first UG found
		bool anyUG = false;
		vector firstSize;
		float firstAcc = 1.0;
		float firstInterp = 1.0;
		bool seeded = false;

		foreach (EditorObject eo : editor_objects)
		{
			UGTriggerObject ug = UGTriggerObject.Cast(eo.GetWorldObject());
			if (!ug) continue;
			anyUG = true;

			if (!seeded)
			{
				firstSize = ug.GetSize();
				UndergroundTrigger trig = ug.GetLinkedTrigger();
				if (trig)
				{
					firstAcc   = Math.Clamp(trig.m_Accommodation, 0.0, 1.0);
					firstInterp= Math.Clamp(trig.m_InterpolationSpeed, 0.0, 1.0);
				}
				seeded = true;
			}
		}

		if (!anyUG) return;

		UG_SizeVec          = firstSize;
		UG_EyeAccommodation = firstAcc;
		UG_Interpolation    = firstInterp;

		if (UG_EyeAccommodation >= 0.5)
			UG_Type = 0;
		else
			UG_Type = 1;

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
		// keep default behavior
		super.PropertyChanged(property_name);

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
			// Set the default EyeAccommodation when Type changes; user can still tweak after
			if (UG_Type == 0) UG_EyeAccommodation = 1.0; // Outer
			else if (UG_Type == 1) UG_EyeAccommodation = 0.0; // Inner
			else if (UG_Type == 2) UG_EyeAccommodation = 0.0; // Transitional

			if (trig) trig.m_Accommodation = UG_EyeAccommodation;
			return;
		}

		if (property_name == "UG_EyeAccommodation")
		{
			UG_EyeAccommodation = Math.Clamp(UG_EyeAccommodation, 0.0, 1.0);
			if (trig) trig.m_Accommodation = UG_EyeAccommodation;
			return;
		}

		if (property_name == "UG_Interpolation")
		{
			UG_Interpolation = Math.Clamp(UG_Interpolation, 0.0, 1.0);
			if (trig) trig.m_InterpolationSpeed = UG_Interpolation;
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
}
