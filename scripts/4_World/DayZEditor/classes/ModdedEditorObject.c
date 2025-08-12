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