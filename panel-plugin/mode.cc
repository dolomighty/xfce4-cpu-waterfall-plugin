/*  mode.cc
 *  Part of xfce4-cpuheatmap-plugin
 *
 *  Copyright (c) Alexander Nordfelth <alex.nordfelth@telia.com>
 *  Copyright (c) gatopeich <gatoguan-os@yahoo.com>
 *  Copyright (c) 2007-2008 Angelo Arrifano <miknix@gmail.com>
 *  Copyright (c) 2007-2008 Lidiriel <lidiriel@coriolys.org>
 *  Copyright (c) 2010 Florian Rivoal <frivoal@gmail.com>
 *  Copyright (c) 2021 Jan Ziak <0xe2.0x9a.0x9b@xfce.org>
 *
 *  This program is free software; you can redistribute it and/or modify
 *  it under the terms of the GNU General Public License as published by
 *  the Free Software Foundation; either version 2 of the License, or
 *  (at your option) any later version.
 *
 *  This program is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *  GNU Library General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along
 *  with this program; if not, write to the Free Software Foundation, Inc.,
 *  51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.
 */

/* The fixes file has to be included before any other #include directives */
#include "xfce4++/util/fixes.h"
#include <cairo/cairo.h>
#include <math.h>
#include <stdlib.h>
#include "mode.h"

struct Point
{
    gfloat x, y;
    Point() : x(0), y(0) {}
    Point(gfloat _x, gfloat _y) : x(_x), y(_y) {}
};

static xfce4::RGBA
mix_colors (gdouble ratio, const xfce4::RGBA &color1, const xfce4::RGBA &color2)
{
    return color1 + ratio * (color2 - color1);
}







#include <assert.h>

static xfce4::RGBA
lerp_rgb ( xfce4::RGBA *colors, int n, float ratio )
{
    assert((int)1.1==(int)floor(1.1));

    if(ratio<0)ratio=0;
    if(ratio>1)ratio=1;

    n--;
    ratio *= n;
    int i1 = ratio;
    ratio -= i1;

    int i0 = i1;
    i1++; if(i1>n)i1=n;

    return mix_colors(ratio,colors[i0],colors[i1]);
//    return colors[i1];
}






void
get_surf_and_patt(
    cairo_surface_t **surf_ptr,
    cairo_pattern_t **patt_ptr,
    int w,
    int h,
    xfce4::RGBA &bg
){
    // statics are 0-filled @ compile time
    static cairo_surface_t *surf;
    static cairo_pattern_t *patt;
    static int pre_w=-1;
    static int pre_h=-1;
    
    assert(surf_ptr);
    assert(patt_ptr);

    // vediamo se qualcosa è cambiato
    if(pre_w==w && pre_h==h && surf && patt){
        // tutto come prima
        *surf_ptr=surf;
        *patt_ptr=patt;
        return;
    }

    // qualcosa è cambiato, rebuild

    pre_w=w;
    pre_h=h;

    if(patt)cairo_pattern_destroy(patt);
    if(surf)cairo_surface_destroy(surf);

    surf=cairo_image_surface_create(CAIRO_FORMAT_RGB24,w,h);
    cairo_t *cr = cairo_create(surf);
    cairo_set_source_rgb(cr,bg.R,bg.G,bg.B);
    cairo_paint(cr);
    cairo_destroy(cr);
    patt=cairo_pattern_create_for_surface(surf);
    cairo_pattern_set_extend(patt,CAIRO_EXTEND_REPEAT);

    *surf_ptr=surf;
    *patt_ptr=patt;
}





void
draw_graph_heatmap (const Ptr<CPUHeatmap> &base, cairo_t *cr, gint w, gint h)
{
    // da quello che ho capito, questa release del plugin
    // chiama una o più volte la draw_*, una volta per core (pseudocore in caso di avg)
    // questa modalità però disegna sempre tutti i cores
    // il resto del codice dovrebbe assicurarsi di chiamare questa
    // solo per il core 0... ma se non lo fa, evitiamo di disegnare

    const int cores = base->history.data.size();

    // bars:
    // past            now
    // |average----------|
    // |core1------------|
    // |core2------------|
    // : : :

    static cairo_matrix_t mat;
    static int x;
    if(mat.xx==0){
        // come trig-once usiamo il fatto che
        // mat non è un'identità
        mat.xx=1;
        mat.yy=1;
    }

    cairo_surface_t *surf;
    cairo_pattern_t *patt;
    get_surf_and_patt(&surf,&patt,w,h,base->colors[BG_COLOR]);

    int stride = cairo_image_surface_get_stride(surf);
    unsigned char *bgra_pixmap = cairo_image_surface_get_data(surf);
    cairo_surface_flush(surf);

    for( int core=0; core<cores; core++ ) 
    {
        // leggiamo solo l'ultimo valore del core
        const CpuLoad *data = base->history.data[core];
        const gssize mask = base->history.mask();
        const int off = base->history.offset;

        xfce4::RGBA c = lerp_rgb(base->colors, NUM_COLORS, data[off&mask].value);
        const int b = 255*c.B;
        const int g = 255*c.G;
        const int r = 255*c.R;

        // disegnamo un segmento verticale
        int y0=h*(core+0)/cores;
        int y1=h*(core+1)/cores;
        
        unsigned char *bgr = &bgra_pixmap[y0*stride+x*4];

        for( int y=y0; y<y1; y++, bgr+=stride ){
            bgr[0]=b;
            bgr[1]=g;
            bgr[2]=r;
        }
    }

    cairo_surface_mark_dirty(surf);

    // ruotiamo il pattern
    x=(x+1)%w;
    mat.x0=x;

    cairo_pattern_set_matrix(patt,&mat);
    cairo_set_source(cr,patt);
    cairo_paint(cr);

    // il destroy non dovrebbe esser importante
    // è implicito al termine del plugin

    // ok, via surface è DECISAMENTE più performante rispetto a stroke
    // tipo almeno il doppio

    // ora proviamo via pattern
    // noi nella surface disegnamo una colonna e lasciamo invariato il resto
    // il pattern repeat (via hw) fa il resto

    // si sembra performare marginalmente meglio
}



