using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;

namespace DatagridControls
{
	/// <summary>
	/// Summary description for UserControl1.
	/// </summary>
	public class djbDataGrid : System.Windows.Forms.DataGrid
	{
		/// <summary>
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public djbDataGrid()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			// TODO: Add any initialization after the InitComponent call

		}

		/// <summary>
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if( components != null )
					components.Dispose();
			}
			base.Dispose( disposing );
		}

		#region Component Designer generated code
		/// <summary>
		/// Required method for Designer support - do not modify 
		/// the contents of this method with the code editor.
		/// </summary>
		private void InitializeComponent()
		{
			// 
			// djbDataGrid
			// 
			this.Name = "djbDataGrid";

		}
		#endregion

		public int VerticalPosition
		{
			get
			{
				return base.VertScrollBar.Value;
			}
			set
			{
				base.VertScrollBar.Value = value;
				base.GridVScrolled(this, 
					new System.Windows.Forms.ScrollEventArgs(System.Windows.Forms.ScrollEventType.ThumbPosition, 
					value));

			}
		}
	}
}
