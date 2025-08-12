class UGTriggerObject : Building
{
	protected vector m_Size;
	protected UndergroundTrigger m_UndergroundTrigger;
	protected ref Timer m_SyncTimer;
	protected bool   m_RescanQueued;
	protected vector m_LastPosePos;
	protected vector m_LastPoseOri;
	protected int m_DesiredUGType = -1;
	
	// ----- Type (0=Outer, 1=Inner, 2=Transitional) -----
	int GetUGType()
	{
		if (m_DesiredUGType >= 0) return m_DesiredUGType;
		UndergroundTrigger trig = GetLinkedTrigger();

		if (trig) return trig.m_Type; 
		return 0;
	}

	void SetUGType(int type)
	{
		type = Math.Clamp(type, 0, 2);
		m_DesiredUGType = type;

		UndergroundTrigger trig = GetLinkedTrigger();
		if (!trig) return;

		// Apply default EyeAccommodation 
		if (type == 0)       SetEyeAccommodation(1.0); // Outer
		else /* 1 or 2 */    SetEyeAccommodation(0.0); // Inner/Transitional

		// Only Transitional supports breadcrumbs —
		if (type != 2 && trig.m_Data)
		{
			trig.m_Data.Breadcrumbs = null;
		}
		trig.m_Type = type;
	}

	void SetEyeAccommodation(float v)
	{
		v = UG_Round2(v);
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig)
		{
			trig.m_Accommodation = v;
			if (m_DesiredUGType == 2)
				trig.m_Type = EUndergroundTriggerType.TRANSITIONING;
		}
	}

	float GetEyeAccommodation()
	{
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) return trig.m_Accommodation;
		return 1.0; 
	}

	void SetInterpolation(float v)
	{
		v = Math.Clamp(v, 0.0, 1.0);
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) { trig.m_InterpolationSpeed = v; }
	}

	float GetInterpolation()
	{
		UndergroundTrigger trig = GetLinkedTrigger();
		if (trig) return trig.m_InterpolationSpeed;
		return 1.0;
	}

	void UGTriggerObject()
	{
		m_Size = Vector(1,1,1);
		ApplySizeTransform();

		CreateTriggerIfMissing();
		UpdateTrigger(); 

		m_LastPosePos = GetPosition();
   		m_LastPoseOri = GetOrientation();
		m_SyncTimer = new Timer(CALL_CATEGORY_SYSTEM);
		m_SyncTimer.Run(0.05, this, "UpdateTriggerPoseOnly", null, true);
	}

	void ~UGTriggerObject()
	{
		if (m_SyncTimer) m_SyncTimer.Stop();
		if (m_UndergroundTrigger && !m_UndergroundTrigger.IsSetForDeletion())
			m_UndergroundTrigger.Delete();
	}

	void SetSize(vector sizeMeters)
	{
		vector s = sizeMeters;
		if (s[0] <= 0.001) s[0] = 0.001;
		if (s[1] <= 0.001) s[1] = 0.001;
		if (s[2] <= 0.001) s[2] = 0.001;
		m_Size = s;
		ApplySizeTransform();
		UpdateTrigger();
		QueueCrumbRescan();
	}

	void TrigSize(float dx, float dy, float dz)
	{
		m_Size[0] = Math.Max(m_Size[0] + dx, 0.01);
		m_Size[1] = Math.Max(m_Size[1] + dy, 0.01);
		m_Size[2] = Math.Max(m_Size[2] + dz, 0.01);
		ApplySizeTransform();
		UpdateTrigger(); 
		QueueCrumbRescan();
	}

	void SetTransformData()
	{
		vector m[4];
		GetTransform(m);
		m_Size = Vector(m[0].Length(), m[1].Length(), m[2].Length());
		UpdateTrigger(); 
	}

	vector GetSize() { return m_Size; }
	UndergroundTrigger GetLinkedTrigger() { return m_UndergroundTrigger; }

	protected void CreateTriggerIfMissing()
	{
		if (m_UndergroundTrigger) return;

		m_UndergroundTrigger = UndergroundTrigger.Cast(GetGame().CreateObjectEx("UndergroundTrigger", GetPosition(), ECE_LOCAL));
		if (!m_UndergroundTrigger)
		{
			Print("[UGTriggerObject] Failed to spawn UndergroundTrigger");
			return;
		}


		m_UndergroundTrigger.m_Accommodation      = 1.0; 
		m_UndergroundTrigger.m_InterpolationSpeed = 1.0;
		m_UndergroundTrigger.m_Type               = EUndergroundTriggerType.OUTER;
		m_DesiredUGType = -1;
		m_UndergroundTrigger.SetPosition(GetPosition());
		m_UndergroundTrigger.SetOrientation(GetOrientation());
	}

	protected bool IsPointInsideOBB(vector p, out vector right, out vector up, out vector fwd, out vector pos, out vector half)
	{
		vector T[4];
		GetTransform(T);
		right = T[0].Normalized();
		up    = T[1].Normalized();
		fwd   = T[2].Normalized();
		pos   = T[3];
		half  = m_Size * 0.5;

		vector d = p - pos;
		float lx = d * right;
		float ly = d * up;
		float lz = d * fwd;

		return (Math.AbsFloat(lx) <= half[0] + 1e-3 && Math.AbsFloat(ly) <= half[1] + 1e-3 && Math.AbsFloat(lz) <= half[2] + 1e-3);
	}

	void GetBreadcrumbs()
	{
		Print("[EditorUGTriggers] GetBreadcrumbs: " + this);

		UndergroundTrigger t = GetLinkedTrigger();
		if (!t) { CreateTriggerIfMissing(); t = GetLinkedTrigger(); if (!t) { Print("[UG][ERR] no trigger; abort"); return; } }

		// Only Transitional collects crumbs; otherwise clear and exit
		if (GetUGType() != 2)
		{
			if (t.m_Data) { t.m_Data.Breadcrumbs = null; }
			Print("[EditorUGTriggers] Not Transitional; cleared crumbs");
			return;
		}

		ref array<Object> results = new array<Object>();
		float r = Math.Max(Math.Max(m_Size[0], m_Size[1]), m_Size[2]) * 1.5;
		GetGame().GetObjectsAtPosition3D(GetPosition(), r, results, null);

		ref array<ref JsonUndergroundAreaBreadcrumb> crumbs = new array<ref JsonUndergroundAreaBreadcrumb>();

		vector right, up, fwd, pos, half;
		foreach (Object obj : results)
		{
			if (!obj) continue;
			if (obj.GetType() != "UGBreadcrumb") continue;

			vector wp = obj.GetPosition();
			if (!IsPointInsideOBB(wp, right, up, fwd, pos, half)) continue;

			UGBreadcrumb crumb = UGBreadcrumb.Cast(obj);
			JsonUndergroundAreaBreadcrumb bc = new JsonUndergroundAreaBreadcrumb();
			bc.Position = new array<float>();
			bc.Position.Insert(wp[0]); bc.Position.Insert(wp[1]); bc.Position.Insert(wp[2]);
			if (crumb) {
				bc.EyeAccommodation = crumb.GetEyeAccommodation();
				bc.UseRaycast = crumb.GetUseRaycast();
				bc.Radius    = crumb.GetRadius();

			} else {
				bc.EyeAccommodation = 1.0;
				bc.UseRaycast = 0;
				bc.Radius    = -1.0;

			}

			crumbs.Insert(bc);
		}

		if (!t.m_Data)
		{
			JsonUndergroundAreaTriggerData d = BuildJsonFromUG(this);
			t.Init(d);
		}

		if (crumbs.Count() >= 2)
		{
			t.m_Data.Breadcrumbs = crumbs;

			if (m_DesiredUGType == 2)
				t.m_Type = EUndergroundTriggerType.TRANSITIONING;

			Print("[EditorUGTriggers] attached " + crumbs.Count() + " breadcrumbs");
		}
		else
		{
			t.m_Data.Breadcrumbs = null;
			Print("[EditorUGTriggers][WARN] need >=2 breadcrumbs; cleared breadcrumb mode");
		}
	}

	void CollectBreadcrumbs()
	{
		UndergroundTrigger t = GetLinkedTrigger();
		if (!t) return;

		if (GetUGType() != 2)
		{
			if (t.m_Data) { t.m_Data.Breadcrumbs = null; }
			return;
		}

		ref array<Object> objs = new array<Object>();
		float r = Math.Max(Math.Max(m_Size[0], m_Size[1]), m_Size[2]) * 1.5;
		GetGame().GetObjectsAtPosition3D(GetPosition(), r, objs, null);

		ref array<ref JsonUndergroundAreaBreadcrumb> crumbs = new array<ref JsonUndergroundAreaBreadcrumb>();
		vector right, up, fwd, pos, half;

		for (int i = 0; i < objs.Count(); i++)
		{
			Object o = objs[i];
			if (!o) continue;
			if (o.GetType() != "UGBreadcrumb") continue;

			vector p = o.GetPosition();
			if (!IsPointInsideOBB(p, right, up, fwd, pos, half)) continue;

			UGBreadcrumb crumb = UGBreadcrumb.Cast(o);
			JsonUndergroundAreaBreadcrumb bc = new JsonUndergroundAreaBreadcrumb();
			bc.Position = new array<float>();
			bc.Position.Insert(p[0]); bc.Position.Insert(p[1]); bc.Position.Insert(p[2]);
			if (crumb) {
				bc.EyeAccommodation = crumb.GetEyeAccommodation();
				bc.UseRaycast = crumb.GetUseRaycast();
				bc.Radius    = crumb.GetRadius();
			} else {
				bc.EyeAccommodation = 1.0;
				bc.UseRaycast = 0;
				bc.Radius    = -1.0;
			}

			crumbs.Insert(bc);
		}

		if (!t.m_Data) t.m_Data = new JsonUndergroundAreaTriggerData();

		if (crumbs.Count() >= 2)
		{
			t.m_Data.Breadcrumbs = crumbs;

			// are you sure?
			if (m_DesiredUGType == 2)
				t.m_Type = EUndergroundTriggerType.TRANSITIONING;
		}
		else
		{
			t.m_Data.Breadcrumbs = null;
		}
	}

	void QueueCrumbRescan()
	{
	    if (m_RescanQueued) return;
	    m_RescanQueued = true;
	    GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(this.DoCrumbRescan, 100, false);
	}

	void DoCrumbRescan()
	{
	    m_RescanQueued = false;
	    if (GetUGType() == 2) { CollectBreadcrumbs(); }
	}

	protected float UG_MapScaleToEyeAcco(float sc)
	{
		if (sc < 0.0) sc = 0.0;
		if (sc > 1.0) sc = 1.0;
		sc = Math.Round(sc * 100.0) / 100.0;
		return sc;
	}

	protected void UpdateTriggerPoseOnly()
	{
		if (!m_UndergroundTrigger) return;

		vector P[4];
		GetTransform(P);

		vector ax0 = P[0].Normalized();
		vector ax1 = P[1].Normalized();
		vector ax2 = Math3D.CrossProduct(ax0, ax1); ax2.Normalize();
		ax1 = Math3D.CrossProduct(ax2, ax0);        ax1.Normalize();

		vector M[4];
		M[0] = ax0;
		M[1] = ax1;
		M[2] = ax2;
		M[3] = GetPosition();

		m_UndergroundTrigger.SetTransform(M);
	}

	protected void UpdateTriggerExtentsOnly()
	{
		if (!m_UndergroundTrigger) return;
		vector e = m_Size * 0.5;
		m_UndergroundTrigger.SetExtents(-e, e);
		m_UndergroundTrigger.SetPosition(m_UndergroundTrigger.GetPosition());
	}

	protected void UpdateTrigger()
	{
		CreateTriggerIfMissing();
		UpdateTriggerPoseOnly();
		UpdateTriggerExtentsOnly();
	}

	protected void ApplySizeTransform()
	{
		vector T[4];
		GetTransform(T);

		vector ax0 = T[0].Normalized();
		vector ax1 = T[1].Normalized();
		vector ax2 = T[2].Normalized();

		T[0] = ax0 * m_Size[0];
		T[1] = ax1 * m_Size[1];
		T[2] = ax2 * m_Size[2];

		SetTransform(T);
	}
}
void UG_RescanTriggersAround(vector center, float radius)
{
    ref array<Object> objs = new array<Object>();
    GetGame().GetObjectsAtPosition3D(center, radius, objs, null);
    for (int i = 0; i < objs.Count(); i++) {
        UGTriggerObject ug = UGTriggerObject.Cast(objs[i]);
        if (!ug) continue;
        if (ug.GetUGType() != 2) continue;
        ug.QueueCrumbRescan();
    }
}

float UG_Round2(float v)
{
	v = Math.Clamp(v, 0.0, 1.0);
	return Math.Round(v * 100.0) / 100.0;
}

//IMPORT to Editor
class UGTriggerApplyRec
{
    vector Pos;
    vector Size;
    float  EyeAcc;
    float  Interp;
    int    Type;  
}

class UGBreadcrumbApplyRec
{
    vector Pos;
    float  EyeAcc;
    int    UseRaycast;
    float  Radius;
}

ref array<ref UGTriggerApplyRec>     g_UG_ToApply = new array<ref UGTriggerApplyRec>();
ref array<ref UGBreadcrumbApplyRec>  g_BC_ToApply = new array<ref UGBreadcrumbApplyRec>();

class UG_PostImportApplier
{
    static int s_Attempts = 20;

    static void Start()
    {
        s_Attempts = 20;
        GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UG_PostImportApplier.Tick, 200, false);
    }

    static void Tick()
    {
        for (int i = g_UG_ToApply.Count() - 1; i >= 0; i--)
        {
            UGTriggerApplyRec rec = g_UG_ToApply[i];
            UGTriggerObject ug = FindNearestUG(rec.Pos, 3.0);
            if (!ug) continue;

            // Apply in correct order
            ug.SetUGType(rec.Type);
            ug.SetSize(rec.Size);
            ug.SetEyeAccommodation(rec.EyeAcc);
            ug.SetInterpolation(rec.Interp);

            if (rec.Type == 2) ug.QueueCrumbRescan();

            g_UG_ToApply.Remove(i);
        }

        for (int j = g_BC_ToApply.Count() - 1; j >= 0; j--)
        {
            UGBreadcrumbApplyRec bc = g_BC_ToApply[j];
            UGBreadcrumb obj = FindNearestBC(bc.Pos, 3.0);
            if (!obj) continue;

            obj.SetEyeAccommodation(bc.EyeAcc);
            obj.SetUseRaycast(bc.UseRaycast);
            obj.SetRadius(bc.Radius);

            g_BC_ToApply.Remove(j);
        }

        if (g_UG_ToApply.Count() == 0 && g_BC_ToApply.Count() == 0) {
            return;
        }

        s_Attempts--;
        if (s_Attempts > 0) {
            GetGame().GetCallQueue(CALL_CATEGORY_SYSTEM).CallLater(UG_PostImportApplier.Tick, 200, false);
        }
    }

    static UGTriggerObject FindNearestUG(vector pos, float radius)
    {
        ref array<Object> objs = new array<Object>();
        GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

        float bestD2 = 1e12;
        UGTriggerObject best;
        foreach (Object o : objs)
        {
            UGTriggerObject ug = UGTriggerObject.Cast(o);
            if (!ug) continue;
            float d2 = vector.DistanceSq(ug.GetPosition(), pos);
            if (d2 < bestD2) { bestD2 = d2; best = ug; }
        }
        return best;
    }

    static UGBreadcrumb FindNearestBC(vector pos, float radius)
    {
        ref array<Object> objs = new array<Object>();
        GetGame().GetObjectsAtPosition3D(pos, radius, objs, null);

        float bestD2 = 1e12;
        UGBreadcrumb best;
        foreach (Object o : objs)
        {
            UGBreadcrumb bc = UGBreadcrumb.Cast(o);
            if (!bc) continue;
            float d2 = vector.DistanceSq(bc.GetPosition(), pos);
            if (d2 < bestD2) { bestD2 = d2; best = bc; }
        }
        return best;
    }
}