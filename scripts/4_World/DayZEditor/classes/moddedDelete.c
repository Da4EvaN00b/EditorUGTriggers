modded class EditorObjectManagerModule
{
    override void DeleteObject(notnull EditorObject target)
    {
        if (target && target.IsVisible() && !target.IsLocked()) {
            UGUndoCache.RememberEO(target);
        }

        super.DeleteObject(target);
    }
}