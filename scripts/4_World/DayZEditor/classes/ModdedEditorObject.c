modded class EditorObject
{
    //Remove Bounding box for UGTriggerObject
    override void ShowBoundingBox()
    {
        Object w = GetWorldObject();
        if (UGTriggerObject.Cast(w))
        {
            return;
        }
        super.ShowBoundingBox();
    }
}