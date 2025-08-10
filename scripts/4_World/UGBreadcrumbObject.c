class UGBreadcrumb : Building
{
    protected float m_UG_EyeAccommodation = 1.0; // 0..1
    protected int   m_UG_UseRaycast = 0;         // 0/1
    protected float m_UG_Radius = -1.0;          // -1 = default
    protected ref Timer m_BCWatch;
    protected vector    m_LastPos;

    void UGBreadcrumb()
    {
        m_LastPos = GetPosition();
        m_BCWatch = new Timer(CALL_CATEGORY_SYSTEM);
        m_BCWatch.Run(0.10, this, "BC_Poll", null, true);
    }
    
    void ~UGBreadcrumb()
    {
        if (m_BCWatch) m_BCWatch.Stop();
    }

    void BC_Poll()
    {
    vector p = GetPosition();
    if (vector.Distance(p, m_LastPos) > 0.001) 
        {
        m_LastPos = p;
        UG_RescanTriggersAround(p, 200.0); // reasonable radius; adjust if needed
        }
    }

    void SetEyeAccommodation(float v)
    {
        if (v < 0.0) v = 0.0;
        if (v > 1.0) v = 1.0;
        m_UG_EyeAccommodation = v;
    }
    float GetEyeAccommodation() { return m_UG_EyeAccommodation; }

    void SetUseRaycast(int v) { m_UG_UseRaycast = v != 0; }
    int  GetUseRaycast() { return m_UG_UseRaycast; }

    void SetRadius(float r) { m_UG_Radius = r; }
    float GetRadius() { return m_UG_Radius; }

};