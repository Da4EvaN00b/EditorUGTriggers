class UGTriggerObject : Building
{
	protected vector m_Size; // desired WORLD size X,Y,Z (meters)
	protected UndergroundTrigger m_UndergroundTrigger;
	protected ref Timer m_SyncTimer;

	void UGTriggerObject()
	{
		m_Size = Vector(1,1,1);
		ApplySizeTransform();

		CreateTriggerIfMissing();
		UpdateTrigger(); // pose + extents

		// Simple 20 Hz pose sync so rotations/moves are reflected immediately
		m_SyncTimer = new Timer(CALL_CATEGORY_SYSTEM);
		m_SyncTimer.Run(0.05, this, "UpdateTriggerPoseOnly", null, true);
	}

	void ~UGTriggerObject()
	{
		if (m_SyncTimer) m_SyncTimer.Stop();
		if (m_UndergroundTrigger && !m_UndergroundTrigger.IsSetForDeletion())
			m_UndergroundTrigger.Delete();
	}

	void SetSize(vector s)
	{
		m_Size = s;
		ApplySizeTransform();
		UpdateTrigger(); // pose + extents
	}

	void TrigSize(float dx, float dy, float dz)
	{
		m_Size[0] = Math.Max(m_Size[0] + dx, 0.01);
		m_Size[1] = Math.Max(m_Size[1] + dy, 0.01);
		m_Size[2] = Math.Max(m_Size[2] + dz, 0.01);
		ApplySizeTransform();
		UpdateTrigger(); // pose + extents
	}

	void SetTransformData()
	{
		vector m[4];
		GetTransform(m);
		m_Size = Vector(m[0].Length(), m[1].Length(), m[2].Length());
		UpdateTrigger(); // pose + extents
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

		m_UndergroundTrigger.m_Accommodation      = 1; // OUTER
		m_UndergroundTrigger.m_InterpolationSpeed = 1;
		m_UndergroundTrigger.m_Type               = EUndergroundTriggerType.OUTER;

		m_UndergroundTrigger.SetPosition(GetPosition());
		m_UndergroundTrigger.SetOrientation(GetOrientation());
	}

	protected void UpdateTriggerPoseOnly()
	{
		if (!m_UndergroundTrigger) return;

		vector P[4];
		GetTransform(P);

		// Strip scale
		vector ax0 = P[0].Normalized();
		vector ax1 = P[1].Normalized();

		// Rebuild orthonormal basis
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
		m_UndergroundTrigger.SetPosition(m_UndergroundTrigger.GetPosition()); // refresh on some builds
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