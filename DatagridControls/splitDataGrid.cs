using System;
using System.Collections;
using System.ComponentModel;
using System.Drawing;
using System.Data;
using System.Windows.Forms;
using System.Reflection;

namespace DatagridControls
{
	/// <summary>
	/// Summary description for splitDataGrid.
	/// </summary>
	public class splitDataGrid : System.Windows.Forms.UserControl
	{
		private DatagridControls.djbDataGrid grdLeft;
		private DatagridControls.djbDataGrid grdRight;
		private System.Windows.Forms.Splitter splitter;
		/// <summary> 
		/// Required designer variable.
		/// </summary>
		private System.ComponentModel.Container components = null;

		public splitDataGrid()
		{
			// This call is required by the Windows.Forms Form Designer.
			InitializeComponent();

			grdRight.RowHeadersVisible = false;

		}

		/// <summary> 
		/// Clean up any resources being used.
		/// </summary>
		protected override void Dispose( bool disposing )
		{
			if( disposing )
			{
				if(components != null)
				{
					components.Dispose();
				}
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
			this.grdLeft = new DatagridControls.djbDataGrid();
			this.splitter = new System.Windows.Forms.Splitter();
			this.grdRight = new DatagridControls.djbDataGrid();
			((System.ComponentModel.ISupportInitialize)(this.grdLeft)).BeginInit();
			((System.ComponentModel.ISupportInitialize)(this.grdRight)).BeginInit();
			this.SuspendLayout();
			// 
			// grdLeft
			// 
			this.grdLeft.DataMember = "";
			this.grdLeft.Dock = System.Windows.Forms.DockStyle.Left;
			this.grdLeft.HeaderForeColor = System.Drawing.SystemColors.ControlText;
			this.grdLeft.Location = new System.Drawing.Point(0, 0);
			this.grdLeft.Name = "grdLeft";
			this.grdLeft.Size = new System.Drawing.Size(232, 184);
			this.grdLeft.TabIndex = 0;
			this.grdLeft.VerticalPosition = 0;
			this.grdLeft.Scroll += new System.EventHandler(this.grdLeft_Scroll);
			// 
			// splitter
			// 
			this.splitter.Location = new System.Drawing.Point(232, 0);
			this.splitter.Name = "splitter";
			this.splitter.Size = new System.Drawing.Size(3, 184);
			this.splitter.TabIndex = 1;
			this.splitter.TabStop = false;
			// 
			// grdRight
			// 
			this.grdRight.DataMember = "";
			this.grdRight.Dock = System.Windows.Forms.DockStyle.Fill;
			this.grdRight.HeaderForeColor = System.Drawing.SystemColors.ControlText;
			this.grdRight.Location = new System.Drawing.Point(235, 0);
			this.grdRight.Name = "grdRight";
			this.grdRight.RowHeadersVisible = false;
			this.grdRight.RowHeaderWidth = 0;
			this.grdRight.Size = new System.Drawing.Size(293, 184);
			this.grdRight.TabIndex = 0;
			this.grdRight.VerticalPosition = 0;
			this.grdRight.Scroll += new System.EventHandler(this.grdRight_Scroll);
			// 
			// splitDataGrid
			// 
			this.Controls.Add(this.grdRight);
			this.Controls.Add(this.splitter);
			this.Controls.Add(this.grdLeft);
			this.Name = "splitDataGrid";
			this.Size = new System.Drawing.Size(528, 184);
			((System.ComponentModel.ISupportInitialize)(this.grdLeft)).EndInit();
			((System.ComponentModel.ISupportInitialize)(this.grdRight)).EndInit();
			this.ResumeLayout(false);

		}
		#endregion

		public int SplitLocation
		{
			get
			{
				return grdLeft.Width;
			}
			set
			{
				grdLeft.Width = value;
			}
		}

		public object DataSource
		{
			get
			{
				return grdRight.DataSource;
			}
			set
			{
				try
				{
					grdLeft.DataSource = value;
					grdRight.DataSource = value;
				}
				catch( Exception e)
				{
					MessageBox.Show(e.Message);
				}
			}
		}

		public System.Windows.Forms.GridTableStylesCollection LeftTableStyles
		{
			get
			{
				return grdLeft.TableStyles;
			}
		}

		public System.Windows.Forms.DataGridTableStyle get_LeftTableStyles(int index)
		{
			return grdLeft.TableStyles[index];
		}
		public System.Windows.Forms.DataGridTableStyle get_LeftTableStyles(string tableName)
		{
			return grdLeft.TableStyles[tableName];
		}
/***************/
		public System.Windows.Forms.GridTableStylesCollection RightTableStyles
		{
			get
			{
				return grdRight.TableStyles;
			}
		}

		public System.Windows.Forms.DataGridTableStyle get_RightTableStyles(int index)
		{
			return grdRight.TableStyles[index];
		}

		private void grdLeft_Scroll(object sender, System.EventArgs e)
		{
			grdRight.VerticalPosition = grdLeft.VerticalPosition;
		}

		private void grdRight_Scroll(object sender, System.EventArgs e)
		{
			grdLeft.VerticalPosition = grdRight.VerticalPosition;
		}
	
		public System.Windows.Forms.DataGridTableStyle get_RightTableStyles(string tableName)
		{
			return grdRight.TableStyles[tableName];
		}

		public bool ReadOnly
		{
			get
			{
				return grdLeft.ReadOnly;
			}
			set
			{
				grdLeft.ReadOnly = value;
				grdRight.ReadOnly = value;
			}
		}

		public void AutoSizeColumn(int colno)
		{
			float width = 0; 
			int numRows = ((DataTable) grdLeft.DataSource).Rows.Count; 
           
			Graphics g = Graphics.FromHwnd(grdLeft.Handle); 
			StringFormat sf = new StringFormat(StringFormat.GenericTypographic); 
			SizeF size; 

			for(int i = 0; i < numRows; ++ i) 
			{ 
				
				size = g.MeasureString(grdLeft[i, colno].ToString(), grdLeft.Font, 500, sf); 
				if(size.Width > width) 
					width = size.Width; 
			} 
			g.Dispose(); 
			if( colno > 1 && width < 70 )
				width = 70;
			grdLeft.TableStyles[0].GridColumnStyles[colno].Width = (int) width + 8; // 8 is for leading and trailing padding 
			if( colno != 0 )
			{
				grdRight.TableStyles[0].GridColumnStyles[colno].Width = (int) width + 8; // 8 is for leading and trailing padding 
				grdLeft.TableStyles[0].GridColumnStyles[colno].Width = (int) width + 8; // 8 is for leading and trailing padding 
			}
			else
			{
				grdRight.TableStyles[0].GridColumnStyles[colno].Width = 0;
				grdLeft.TableStyles[0].GridColumnStyles[colno].Width = 0;
			}
			grdRight.RowHeadersVisible = false;
		}

		public void AutoSizeColumns()
		{
			
		}

		public void SizeRowHeight(int index, int height)
		{
			//AutoSizeRows(index, grdLeft, height);
			//AutoSizeRows(index, grdRight, height);
		}

		public void AutoSizeRows(int index, System.Windows.Forms.DataGrid datagrid, int height)
		{
			// DataGrid should be bound to a DataTable for this part to 
			// work. 
			int numRows = ((DataTable)datagrid.DataSource).Rows.Count; 
//			Graphics g = Graphics.FromHwnd(datagrid.Handle); 
//			StringFormat sf = new StringFormat(StringFormat.GenericTypographic); 
//			SizeF size; 

			// Since DataGridRows[] is not exposed directly by the DataGrid 
			// we use reflection to hack internally to it.. There is actually 
			// a method get_DataGridRows that returns the collection of rows 
			// that is what we are doing here, and casting it to a System.Array 
			MethodInfo mi = datagrid.GetType().GetMethod("get_DataGridRows", 
				BindingFlags.FlattenHierarchy | BindingFlags.IgnoreCase | BindingFlags.Instance 
				| BindingFlags.NonPublic | BindingFlags.Public | BindingFlags.Static ); 

			System.Array dgra = (System.Array)mi.Invoke(datagrid,null); 

			// Convert this to an ArrayList, little bit easier to deal with 
			// that way, plus we can strip out the newrow row. 
			ArrayList DataGridRows = new ArrayList(); 
			foreach (object dgrr in dgra) 
			{ 
				if (dgrr.ToString().EndsWith("DataGridRelationshipRow")==true) 
					DataGridRows.Add(dgrr); 
			} 

			// Now we pick that row out of the DataGridRows[] Array 
			// that we have and set it's Height property to what we 
			// think it should be. 
			PropertyInfo pi = DataGridRows[index].GetType().GetProperty("Height"); 
			pi.SetValue(DataGridRows[index],height,null); 
			
			// Now loop through all the rows in the grid 
//			for (int i = 0; i < numRows; ++i) 
//			{ 
				// Here we are telling it that the column width is set to 
				// 400.. so size will contain the Height it needs to be. 
//				size = g.MeasureString(grdLeft[i,1].ToString(),gridTasks.Font,400,sf); 
//				int h = Convert.ToInt32(size.Height); 
				// Little extra cellpadding space 
//				h = h + 8; 


				// I have read here that after you set the Height in this manner that you should 
				// Call the DataGrid Invalidate() method, but I haven't seen any prob with not calling it.. 

			}


//			g.Dispose(); 
//		} 
/***************/

	}
}
