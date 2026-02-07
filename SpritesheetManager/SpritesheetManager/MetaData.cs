using System;
using System.Collections.Generic;
using Gtk;
using Gdk;

namespace SpritesheetManager
{
    public enum SpriteType
    {
        Spritesheet, // default
        Cubemap,
        Animated
    };

    public class MetaData
    {
        public string relativepath { get; set; }
        public int id { get; set; }
        public Rectangle rect { get; set; }
        public SpriteType spritetype { get; set; }
        public int frame { get; set; }

        public bool IsValid(List<MetaData> data)
        {
            int currentid = 0;
            foreach (var a in data)
            {
                if (a.id > currentid)
                {
                    currentid = a.id;
                }
                if (a.spritetype == SpriteType.Cubemap && a.frame > 6) //validates cubemaps cant have more than 6 faces
                {
                    MessageDialog md = new MessageDialog(
                        MainClass.win,
                        DialogFlags.Modal,
                        MessageType.Error,
                        ButtonsType.Ok,
                        $"an cube map spritesheet contained more than 6 elements (faces) hashcode : {a.GetHashCode()}"
                    );
                    md.Run();
                    md.Destroy();
                    return false;
                }
            }
            return true;
        }
    }
}
