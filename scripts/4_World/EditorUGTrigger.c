modded class UndergroundTrigger
{
	void SetAccommodation(float v)
    { 
        m_Accommodation = v;
    }
	void SetInterpolationSpeed(float v)
    { 
        m_InterpolationSpeed = v; 
    } 
    void SetExtentsFromSize(vector size)
	{
		vector e = size * 0.5;
		SetExtents(-e, e);
	}

    //Over ride the existing trigger enter and leave events to call the client handler because the Server Handler was set to only run in dev mode, but we need it. 
	override protected void OnEnterServerEvent(TriggerInsider insider) 
	{
		OnEnterClientEvent(insider);
	}
	override protected void OnLeaveServerEvent(TriggerInsider insider) 
	{
		OnLeaveClientEvent(insider);
	}
	
	override protected void OnEnterClientEvent(TriggerInsider insider) 
	{
		PlayerBase player = PlayerBase.Cast(insider.GetObject());
		if (player)
		{
			UndergroundHandlerClient handler = player.GetUndergroundHandler();
			if (handler)
			{
				handler.OnTriggerEnter(this);
			}
		}
	}
	
	override protected void OnLeaveClientEvent(TriggerInsider insider) 
	{
		PlayerBase player = PlayerBase.Cast(insider.GetObject());
		if (player)
		{
			UndergroundHandlerClient handler = player.GetUndergroundHandler();
			if (handler)
			{
				handler.OnTriggerLeave(this);
			}
		}
	}

    void SetTypeForAccommodation()
    {
    if (m_Accommodation == 1)
        {
            m_Type = EUndergroundTriggerType.OUTER;
        }
    else
        {
            m_Type = EUndergroundTriggerType.INNER;
        }
    }

}

modded class EditorObject
{
    //Remove Bounding box
    override void EnableBoundingBox(bool enable)
    {
        Object w = GetWorldObject();
        if (UGTriggerObject.Cast(w))
        {
            DestroyBoundingBox();
            return;
        }
        super.EnableBoundingBox(enable);
    }
}
modded class EditorGizmo
{
    //removing EditorGizmo for UGTriggerObjects
    override void Update(float dt)
    {
        auto editor = GetEditor();
        if (!editor)
        {
            super.Update(dt);
            return;
        }

        ref array<EditorObject> ordered = editor.GetSelectedObjectsOrdered();
        if (!ordered || ordered.Count() == 0)
        {
            super.Update(dt);
            return;
        }

        EditorObject top = ordered[ordered.Count() - 1];
        bool isUG = top && UGTriggerObject.Cast(top.GetWorldObject());

        if (isUG)
        {
            m_InteractionIndex  = -1;
            m_DragOffset        = vector.Zero;
            m_DragRotationOffset= vector.Zero;

            // Hide the gizmo
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, false);

            return;
        }
        else
        {
            if (m_Gizmo)
                m_Gizmo.SetFlags(EntityFlags.VISIBLE, true);

            super.Update(dt);
        }
    }
}
modded class EditorObjectDragHandler
{
    // Re-apply UGTriggerObject after drag
    override void OnDragging(notnull EditorObject target, notnull array<EditorObject> additional_drag_targets)
    {
        super.OnDragging(target, additional_drag_targets);
        UG_ReapplyNonUniformScaleFor(target);
        foreach (EditorObject eo : additional_drag_targets)
        {
            UG_ReapplyNonUniformScaleFor(eo);
        }
    }

    protected void UG_ReapplyNonUniformScaleFor(EditorObject eo)
    {
        if (!eo) return;

        Object w = eo.GetWorldObject();
        UGTriggerObject ugo = UGTriggerObject.Cast(w);
        if (!ugo) return; 

        vector m[4];
        w.GetTransform(m);

        vector ax0 = m[0].Normalized();
        vector ax1 = m[1].Normalized();
        vector ax2 = m[2].Normalized();

        vector s = ugo.GetSize();
        m[0] = ax0 * s[0];
        m[1] = ax1 * s[1];
        m[2] = ax2 * s[2];

        w.SetTransform(m);
        w.Update(); 
    }
}

//Add to Export File Menu in Edior
modded class EditorCommandManager
{
	override void Init()
	{
        RegisterCommand(ExportUGTriggersToJSON);
        super.Init();    
    }
}

class ExportUGTriggersToJSON: EditorExportCommandBase
{
	override typename GetFileType() 
	{
		return EditorUGTriggerFile;
	}
	
	override string GetName() 
	{
		return "Export UGTriggers (*.json)";
	}
}

class EditorUGTriggerFile : EditorFileType
{
    override void Export(EditorSaveData data, string file, ExportSettings settings, eDialogExtraSetting dialog_setting)
    {
        UGTriggersExportRoot root = new UGTriggersExportRoot();
        foreach (EditorObjectData obj_data : data.EditorObjects)
        {
            UGTriggerObject ug = UGTriggerObject.Cast(obj_data.WorldObject);
            if (!ug) continue;

            vector pos    = obj_data.Position;
            vector orient = obj_data.Orientation;
            vector size   = ug.GetSize();

            float acc    = UG_Round2(ug.GetEyeAccommodation());
            float interp = UG_Round2(ug.GetInterpolation());

            UndergroundTrigger trig = ug.GetLinkedTrigger();

            UGTriggersExport ex = new UGTriggersExport(pos, orient, size, acc, interp);

            if (ug.GetUGType() == 2 && trig && trig.m_Data && trig.m_Data.Breadcrumbs && trig.m_Data.Breadcrumbs.Count() >= 2)
            {
                ex.Breadcrumbs = new array<ref UGBreadcrumbExport>();
                foreach (JsonUndergroundAreaBreadcrumb b : trig.m_Data.Breadcrumbs)
                {
                    UGBreadcrumbExport eb = new UGBreadcrumbExport();
                    eb.Position = new array<float>();
                    eb.Position.Insert(b.Position.Get(0));
                    eb.Position.Insert(b.Position.Get(1));
                    eb.Position.Insert(b.Position.Get(2));
                    eb.EyeAccommodation = UG_Round2(b.EyeAccommodation);
                    eb.UseRaycast       = b.UseRaycast;
                    eb.Radius           = b.Radius;
                    ex.Breadcrumbs.Insert(eb);
                }
            }

            root.Triggers.Insert(ex);
        }

        JsonFileLoader<UGTriggersExportRoot>.JsonSaveFile(file, root);
    }

    override string GetExtension()
	{
		return ".json";
	}

    override void GetValidExtensions(notnull inout array<ref Param2<string, string>> valid_extensions)
	{
		super.GetValidExtensions(valid_extensions);
		valid_extensions.Insert(new Param2<string, string>("Object Spawner", "*.json"));
	}
}

modded class EditorExportMenu
{
    void EditorExportMenu()
    {
        AddMenuButton(m_Editor.CommandManager[ExportUGTriggersToJSON]);
    }
}