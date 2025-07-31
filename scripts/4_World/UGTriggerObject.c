class UGTriggerObject : Building
{
    vector m_Size;

    void UGTriggerObject()
    {
        m_Size = Vector(1, 1, 1);
        ApplySizeTransform(); 
    }

    void SetSize(vector s)
    {
        m_Size = s;
        ApplySizeTransform();
    }

    void SetTransformData()
    {
        vector mat[4];
        GetTransform(mat);
        float sx = mat[0].Length();
        float sy = mat[1].Length();
        float sz = mat[2].Length();
        m_Size = Vector(sx, sy, sz);
    }

    void TrigSize(float dx, float dy, float dz)
    {
        m_Size[0] = Math.Max(m_Size[0] + dx, 0.01);
        m_Size[1] = Math.Max(m_Size[1] + dy, 0.01);
        m_Size[2] = Math.Max(m_Size[2] + dz, 0.01);
        ApplySizeTransform();
    }

    void ApplySizeTransform()
    {
        vector mat[4];
        GetTransform(mat);

        vector orientation[3];
        orientation[0] = mat[0].Normalized();
        orientation[1] = mat[1].Normalized();
        orientation[2] = mat[2].Normalized();

        mat[0] = orientation[0] * m_Size[0];
        mat[1] = orientation[1] * m_Size[1];
        mat[2] = orientation[2] * m_Size[2];

        SetTransform(mat);
    }

    vector GetSize() { return m_Size; }
}

