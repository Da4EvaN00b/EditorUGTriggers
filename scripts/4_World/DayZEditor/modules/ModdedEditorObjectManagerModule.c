modded class EditorObjectManagerModule
{
    void EditorObjectManagerModule(Editor editor)
    {
        EditorPlaceableItem epi;
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("UGTriggerObject", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }
        if (m_PlaceableObjectsByType && m_PlaceableObjectsByType.Find("UGBreadcrumb", epi) && epi) {
            epi.ConsoleFriendly = 1;
        }
    }
}