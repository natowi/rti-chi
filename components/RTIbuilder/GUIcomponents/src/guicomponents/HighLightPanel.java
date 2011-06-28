/*
 *  RTIbuilder
 *  Copyright (C) 2008-11  Universidade do Minho and Cultural Heritage Imaging
 *
 *  This program is free software: you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License version 3 as published
 *  by the Free Software Foundation.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License
 *  along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

package guicomponents;


import java.awt.Graphics;
import java.awt.Graphics2D;



/**Image panel used for image preview and highlight detection representation and edition on the image.<br>
 *By default the image scale on panel is user defined and highlight adjustment is enabled, options that can be changed by the available methods
 *{@link setUserDefinedScale()} and {@link setHighLightEditable()}.
 */
public class HighLightPanel extends PreviewPanel {


	/**Draw the highlight if true.*/
    private boolean enabledraw = false;
	/**Flags if HighLight adjustment is allowed*/
	private boolean enable_edition =  true;
	/**The highlight x coordinate.*/
    private float xc;
	/**The highlight y coordinate.*/
    private float yc;
	/**The highlight coordinates*/
    private float center[] = {0, 0};
    /**The area location where the highlight is represented.*/
	private int area_x = 0,area_y =0;
	/**Flags if the highlight is being moved*/
	private boolean moving_highlight = false;
	/**Flags if the highlight is not detected for this image*/
	private boolean hlt_not_detected = false;
	/**Flags if the highlight was defined by the user*/
	private boolean user_defined =  false;


    /** Creates new form HighLightPanel */
    public HighLightPanel() {
		super();
		super.setUserDefinedScale(true);
        initComponents();
        this.enableHighlightDraw(true);
        this.resize_factor = 1.0f;
        this.repaint();
    }
    /** Creates new form HighLightPanel
	 *@param bi panel buffered image.
	 *@param center the highlight coordinates
	 *@param scale the image drawing scale
	 *@param EnableDraw true if highlight drawing is enable, false otherwise
	 */
    public HighLightPanel(java.awt.image.BufferedImage bi, float center[], float scale, boolean EnableDraw) {
        super();
		user_defined_scale =  true;
		super.setImage(bi);
        initComponents();
        this.center = center;
		this.xc = center[0] * scale;
        this.yc = center[1] * scale;
        this.enableHighlightDraw(EnableDraw);
        this.resize_factor = 1.0f;
        this.repaint();
    }

    /** This method is called from within the constructor to
     * initialize the form.
     * WARNING: Do NOT modify this code. The content of this method is
     * always regenerated by the Form Editor.
     */
      // <editor-fold defaultstate="collapsed" desc="Generated Code">//GEN-BEGIN:initComponents
      private void initComponents() {

            setBorder(javax.swing.BorderFactory.createEtchedBorder());
            setMaximumSize(new java.awt.Dimension(249, 229));
            setMinimumSize(new java.awt.Dimension(249, 229));
            addMouseListener(new java.awt.event.MouseAdapter() {
                  public void mouseClicked(java.awt.event.MouseEvent evt) {
                        formMouseClicked(evt);
                  }
                  public void mousePressed(java.awt.event.MouseEvent evt) {
                        formMousePressed(evt);
                  }
                  public void mouseReleased(java.awt.event.MouseEvent evt) {
                        formMouseReleased(evt);
                  }
            });
            addMouseMotionListener(new java.awt.event.MouseMotionAdapter() {
                  public void mouseDragged(java.awt.event.MouseEvent evt) {
                        formMouseDragged(evt);
                  }
            });
      }// </editor-fold>//GEN-END:initComponents


private void formMouseReleased(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_formMouseReleased

    //If changing the highlight
    if(this.moving_highlight) {
		//Set the center on mouse location
        this.xc = evt.getX() / resize_factor;
        this.yc = evt.getY() / resize_factor;
        this.repaint();
    }
	//set new location
    this.center[0] = this.xc;
    this.center[1] = this.yc;
    
    this.moving_highlight = false;
}//GEN-LAST:event_formMouseReleased

private void formMousePressed(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_formMousePressed
   
	//Distance between mouse point and highlight center
	float xr = this.xc - evt.getX()/resize_factor;
    float yr = this.yc - evt.getY()/resize_factor;

	//If the mouse pointer is near the highlight
   if(this.enable_edition && Math.abs(xr*resize_factor) < 5 && Math.abs(yr*resize_factor) < 5) {
        //Set new highlight
		this.xc = evt.getX() / resize_factor;
        this.yc = evt.getY() / resize_factor;
        //highlight is being moved
		this.moving_highlight =  true;
        this.repaint();
    }

	 //set new location
     this.center[0] = this.xc;
     this.center[1] = this.yc;
}//GEN-LAST:event_formMousePressed

    private void formMouseDragged(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_formMouseDragged

		//If is being moved
        if(this.moving_highlight) {
            //change center
			this.xc = evt.getX() / resize_factor;
            this.yc = evt.getY() / resize_factor;
            this.repaint();
        }

		//set new location
		this.center[0] = this.xc;
		this.center[1] = this.yc;
    }//GEN-LAST:event_formMouseDragged

	private void formMouseClicked(java.awt.event.MouseEvent evt) {//GEN-FIRST:event_formMouseClicked
		  //On double click, set the highlight on mouse location.
		  if (evt.getClickCount() == 2) {
				float x = evt.getX() / resize_factor;
				float y = evt.getY() / resize_factor;
				this.center[0] = x;
				this.center[1] = y;
				this.repaint();
		  }
	}//GEN-LAST:event_formMouseClicked
        
      // Variables declaration - do not modify//GEN-BEGIN:variables
      // End of variables declaration//GEN-END:variables
    
     @Override
	  public void paint(Graphics g) {
			super.paint(g);
			try {
				  if (panel_image != null) {

						//if highlight is not defined
						if (hlt_not_detected) {
							  g.setColor(java.awt.Color.RED);
							  g.drawString("HighLight not defined", 20, this.getHeight() - 20);
						}
						if (enabledraw) {
							  //if highlight is user defined
							  if (!hlt_not_detected&&user_defined) {
									g.setColor(java.awt.Color.RED);
									g.drawString("HighLight defined by user", 20, this.getHeight() - 20);
							  }

							  //set center points
							  xc = center[0];
							  yc = center[1];

							  int x1 = Math.round(xc * resize_factor + 0.5f * resize_factor);
							  int y1 = Math.round(yc * resize_factor + 0.5f * resize_factor);

							  int r = 2;

							  Graphics2D g2D = (Graphics2D) g;
							  g2D.setColor(java.awt.Color.red);
							  g2D.setStroke(new java.awt.BasicStroke(1));

							  //draw highlight
							  g2D.drawLine(x1, y1 - r, x1, y1 - 1);
							  g2D.drawLine(x1, y1 + r, x1, y1 + 1);
							  g2D.drawLine(x1 - r, y1, x1 - 1, y1);
							  g2D.drawLine(x1 + r, y1, x1 + 1, y1);

						}
				  }
			} catch (java.lang.NullPointerException ex) {
				  return;
			}
	  }

    /**Return true if highlight is being drwn on image.
	*@return  true if highlight drawing is enable, false otherwise.
	*/
    public boolean isEnabledraw() {
        return enabledraw;
    }

	/**Enables/Disables highlight drawing on image.<br>
	 *Highlight not define warnings are shown anyway.
	 *@param enabledraw true to enable drawing, false otherwise.
	 */
    public void enableHighlightDraw(boolean enabledraw) {
        this.enabledraw = enabledraw;
    }

	/**Return true if the user can adjust the highlight, false otherwise.
	*@return  true if highlight editing is enabled .
	*/
    public boolean isHighLightEditable() {
        return enable_edition;
    }

	/**Enables/Disables highlight edition.
	 *@param enable_edition true to enable highlight adjustment, false otherwise.
	 */
    public void setHighLightEditable(boolean enable_edition) {
        this.enable_edition = enable_edition;
    }

    /**Return the highlight's coordinates in full size scale.
	*@return  a float array with the x, y coordinates.
	*/
    public float[] getHighLight() {
		//Return the highlight in real world coordinates
        float[] center_float = {(center[0])+area_x,(center[1])+area_y};
        return center_float;
    }

	/**Set the highlight's coordinates.
	*@param c a two size array with x,y coordinates.
	*/
    public void setHighLight(float c[]) {
        this.center = new float[3];
		//convert the real coordinates to area located coordinates.
        this.center[0] = ((c[0])-area_x);
        this.center[1] = ((c[1])-area_y);
    }

	/**Set the image area location. Because the highlight is represented in a image segment, this must be located.
	 *@param the rigt upper corner of the area where the highlight is located
	 */
	public void setImageLocation(int x,int y){
		area_x =x;
		area_y =y;
	}

	/**Set the highlight state as not defined.<br>
	 *A highlight is shown on left upper corner so he can edit and set a new one.
	 *To not show the highlight invoke the {@link enableHighlightDraw(boolean enabledraw)}.
	 *@param hltd true if the highlight was not defined, false otherwise.
	 */
    public void HighLight_Not_Defined(boolean hltd){
		this.hlt_not_detected = hltd;
		//make a fake highlight so the user can define it
		center[0] =5;
		center[1] =5;
	}

	/**Set the highlight state as defined by the user.
	*@param userd true if the highlight was defined by the user, false otherwise
	*/
	public void HighLight_Defined_By_User(boolean userd){
		this.user_defined = userd;
	}

}