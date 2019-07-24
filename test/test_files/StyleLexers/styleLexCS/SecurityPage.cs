//------------------------------------------------------------------------------
// <copyright file="SecurityPage.cs" company="Microsoft">
//     Copyright (c) Microsoft Corporation.  All rights reserved.
// </copyright>
//------------------------------------------------------------------------------

namespace System.Web.Administration {
    using System;
    using System.Collections;
    using System.Web.Security;
    using System.Web.UI;
    using System.Web.UI.WebControls;
    using System.Security;
    using System.Security.Permissions;
    
    [AspNetHostingPermission(SecurityAction.LinkDemand, Level=AspNetHostingPermissionLevel.Minimal)]
    [AspNetHostingPermission(SecurityAction.InheritanceDemand, Level=AspNetHostingPermissionLevel.Minimal)]
    public class SecurityPage : WebAdminPage {
        private const string CURRENT_PATH = "WebAdminCurrentPath";
        
        protected string CurrentPath {
            get {
                object tempString = (object)Session[CURRENT_PATH];
                if (tempString != null) {
                    return (string)Session[CURRENT_PATH];
                }
                return string.Empty;
            }
            set {
                Session[CURRENT_PATH] = value;
            }
        }

        protected override void OnInit(EventArgs e) {
            NavigationBar.SetSelectedIndex(1);
            base.OnInit(e);
        }
    
        protected void SearchForUsers(object sender, EventArgs e, Repeater repeater, GridView dataGrid, DropDownList dropDown, TextBox textBox) {
            ICollection coll = null;
            string text = textBox.Text;
            text = text.Replace("*", "%");
            text = text.Replace("?", "_");
            int total = 0;

            if (text.Trim().Length != 0) {
                if (dropDown.SelectedIndex == 0 /* userID */) {
                    coll = (MembershipUserCollection)CallWebAdminHelperMethod(true, "FindUsersByName", new object[] {(string)text, 0, Int32.MaxValue, total}, new Type[] {typeof(string), typeof(int), typeof(int), Type.GetType("System.Int32&")});
                } else {
                    coll = (MembershipUserCollection)CallWebAdminHelperMethod(true, "FindUsersByEmail", new object[]{(string)text, 0, Int32.MaxValue, total}, new Type[] {typeof(string), typeof(int), typeof(int), Type.GetType("System.Int32&")});
                }
            }

            dataGrid.PageIndex = 0;
            dataGrid.DataSource = coll;
            PopulateRepeaterDataSource(repeater);
            repeater.DataBind();
            dataGrid.DataBind();
        }

    }
}
