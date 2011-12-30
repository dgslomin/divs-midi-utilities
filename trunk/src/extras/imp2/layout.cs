
namespace Sreal.Layout
{
	class VBoxLayoutEngine: System.Windows.Forms.LayoutEngine
	{
		public override bool Layout(object Container, System.Windows.Forms.LayoutEventArgs LayoutEventArgs)
		{
			System.Windows.Forms.Control Container_ = Container as System.Windows.Forms.Control;
			System.Drawing.Point NextControlLocation = Container_.DisplayRectangle.Location;

			foreach (System.Windows.Forms.Control Control in Container_.Controls)
			{
				if (!Control.Visible) continue;
				NextControlLocation.Offset(Control.Margin.Left, Control.Margin.Top);
				Control.Location = NextControlLocation;
				if (Control.AutoSize) Control.Size = Control.GetPreferredSize(Container_.DisplayRectangle.Size);
				NextControlLocation.X = Container_.DisplayRectangle.X;
				NextControlLocation.Y += Control.Height + Control.Margin.Bottom;
			}

			return false;
		}
	}

	class HBoxLayoutEngine: System.Windows.Forms.LayoutEngine
	{
		public override bool Layout(object Container, System.Windows.Forms.LayoutEventArgs LayoutEventArgs)
		{
			System.Windows.Forms.Control Container_ = Container as System.Windows.Forms.Control;
			System.Drawing.Point NextControlLocation = Container_.DisplayRectangle.Location;

			foreach (System.Windows.Forms.Control Control in Container_.Controls)
			{
				if (!Control.Visible) continue;
				NextControlLocation.Offset(Control.Margin.Left, Control.Margin.Top);
				Control.Location = NextControlLocation;
				if (Control.AutoSize) Control.Size = Control.GetPreferredSize(Container_.DisplayRectangle.Size);
				NextControlLocation.X += Control.Width + Control.Margin.Right;
				NextControlLocation.Y = Container_.DisplayRectangle.Y;
			}

			return false;
		}
	}
}

