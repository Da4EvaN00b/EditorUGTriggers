class EditorUGTriggerFile : EditorFileType
{
    //Export UGTriggers to JSON
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

        // - Delete on Export if box is selected
        bool removeUG = UGExportState.RemoveUGAfterExport;
        UGExportState.RemoveUGAfterExport = false; // reset

        if (removeUG) {
            int removed = UG_DeleteAllUGObjects(GetEditor());
            GetEditor().GetEditorHud().CreateNotification(string.Format("[UG Triggers] Exported and removed %1 UG objects", removed));
        } else {
            GetEditor().GetEditorHud().CreateNotification("[UG Triggers] Export complete");
        }
    }
    //Import UGTriggers from JSON
    override EditorSaveData Import(string file, ImportSettings settings)
    {
        EditorSaveData save_data = new EditorSaveData();
        UGTriggersExportRoot import_data = new UGTriggersExportRoot();
        JsonFileLoader<UGTriggersExportRoot>.JsonLoadFile(file, import_data);

        if (!import_data || !import_data.Triggers || import_data.Triggers.Count() == 0) {
            return save_data;
        }

        foreach (UGTriggersExport t : import_data.Triggers)
        {
            vector pos    = Vector(t.Position[0], t.Position[1], t.Position[2]);
            vector orient = Vector(t.Orientation[0], t.Orientation[1], t.Orientation[2]);
            vector size   = Vector(t.Size[0], t.Size[1], t.Size[2]);
            float acc = t.EyeAccommodation;
            acc = UG_Round2(Math.Clamp(acc, 0.0, 1.0));
            float interp = t.InterpolationSpeed;
            interp = UG_Round2(Math.Clamp(interp, 0.0, 1.0));

            // Hacky way to determine Trigger type on import - looking for better ideas.
            int ugType = 0;
            if (t.Breadcrumbs && t.Breadcrumbs.Count() >= 2) {
                ugType = 2;
            } else if (acc <= 0.5) {
                ugType = 1;
            } else {
                ugType = 0;
            }

            UGTriggerApplyRec arec = new UGTriggerApplyRec();
            arec.Pos    = pos;
            arec.Size   = size;
            arec.EyeAcc = acc;
            arec.Interp = interp;
            arec.Type   = ugType;
            g_UG_ToApply.Insert(arec);

            EditorObjectData dta = EditorObjectData.Create("UGTriggerObject", pos, orient, 1.0, EFE_DEFAULT);
            save_data.EditorObjects.Insert(dta);

            if (t.Breadcrumbs)
            {
                foreach (UGBreadcrumbExport be : t.Breadcrumbs)
                {
                    vector bpos    = Vector(be.Position[0], be.Position[1], be.Position[2]);
                    vector borient = Vector(0, 0, 0);

                    UGBreadcrumbApplyRec bcrec = new UGBreadcrumbApplyRec();
                    float bAcc = UG_Round2(Math.Clamp(be.EyeAccommodation, 0.0, 1.0));
                    bcrec.Pos        = bpos;
                    bcrec.EyeAcc     = bAcc;
                    bcrec.UseRaycast = be.UseRaycast;
                    bcrec.Radius     = be.Radius;
                    g_BC_ToApply.Insert(bcrec);

                    EditorObjectData bcDta = EditorObjectData.Create("UGBreadcrumb", bpos, borient, 1.0, EFE_DEFAULT);
                    save_data.EditorObjects.Insert(bcDta);
                }
            }
        }
        
        UG_PostImportApplier.Start();

        return save_data;
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