#include <gtk/gtk.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <linux/input.h>
#define LCD_WIDTH 240
#define LCD_HEIGHT 320
#define LCD_PER_BIT 32
#define LCD_FILE_PATH  "/tmp/lcdMem"
#define KEY_FILE_PATH  "/tmp/keyFifo"

char *g_pLcdBuffer=NULL;
int g_iFdLcd;
int stride;
int fd_fifo;
struct input_event key_event;

gint delete_event( GtkWidget *widget,
		GdkEvent  *event,
		gpointer   data )
{
	return FALSE;
}
	static gboolean
on_expose_event (GtkWidget * widget,
		GdkEventExpose * event, gpointer data)                
{
	cairo_t *cr;
	cairo_surface_t *surface; 
	cr = gdk_cairo_create (widget->window);
	surface = cairo_image_surface_create_for_data(g_pLcdBuffer,CAIRO_FORMAT_RGB24,LCD_WIDTH,LCD_HEIGHT, stride); 
	cairo_set_source_surface(cr, surface, 5, 5); 
	cairo_paint(cr); 
	cairo_destroy (cr);
	return TRUE;
}

void destroy( GtkWidget *widget,
		gpointer   data )
{
	gtk_main_quit ();
}

gint timer_handler( gpointer data )
{
	GtkWidget *widget = (GtkWidget *)data;
	gtk_widget_queue_draw(widget);
	return 1;
}
struct key_map {
	int value;
	int newValue;
}keyMaps[]={
	{111, KEY_UP},
	{113, KEY_LEFT},
	{114, KEY_RIGHT},
	{116, KEY_DOWN},
};

static unsigned int GetKeyValue(int keyvalue)
{
	int i;
	for ( i = 0; i<sizeof(keyMaps)/sizeof(keyMaps[0]); i++){
		if (keyMaps[i].value == keyvalue)
			return keyMaps[i].newValue;
	}
	return 65535;
}

gboolean on_key_callback(GtkWidget *widget,
		GdkEvent  *event,
		gpointer   user_data)
{
	GdkEventKey *pEvent = (GdkEventKey *)event;

	if (pEvent->type == GDK_KEY_PRESS)
		key_event.value = 1;
	else if (pEvent->type == GDK_KEY_RELEASE)
		key_event.value = 0;
	else 
		return TRUE;

	key_event.type = 1;
	key_event.code = GetKeyValue(pEvent->hardware_keycode);
	if (key_event.code == 65535)
		return TRUE;
	write(fd_fifo, &key_event, sizeof(struct input_event));
	fprintf(stderr, "code:%d value:%d\n", key_event.code, 
			key_event.value);

	return TRUE;

}

int main( int   argc,
		char *argv[] )
{
	GtkWidget *window;

	gtk_init (&argc, &argv);

	window = gtk_window_new (GTK_WINDOW_TOPLEVEL);

	g_signal_connect (G_OBJECT (window), "delete_event",
			G_CALLBACK (delete_event), NULL);

	g_signal_connect (G_OBJECT (window), "destroy",
			G_CALLBACK (destroy), NULL);

	gtk_window_set_title (GTK_WINDOW(window), "uc/gui SIMULATION");
	gtk_widget_set_app_paintable(window, TRUE); 
	gtk_window_set_position(GTK_WINDOW(window), GTK_WIN_POS_CENTER); 
	gtk_window_set_default_size(GTK_WINDOW(window), 
			LCD_WIDTH+15, LCD_HEIGHT+15); 

	/* 窗体重画消息 */
	g_signal_connect (G_OBJECT (window), "expose-event",
			G_CALLBACK (on_expose_event), NULL);    

	/* 定时发送重画消息 24FPS*/
	gtk_timeout_add(1000/24, timer_handler, (gpointer)window);

	gtk_widget_show_all (window);

	g_iFdLcd = open(LCD_FILE_PATH,O_CREAT|O_RDWR|O_TRUNC,00777);
	lseek(g_iFdLcd, LCD_WIDTH*LCD_HEIGHT*LCD_PER_BIT, SEEK_SET);
	write(g_iFdLcd," ",1);
	g_pLcdBuffer = (char*) mmap( NULL, LCD_WIDTH*LCD_HEIGHT*LCD_PER_BIT,PROT_READ|PROT_WRITE,
			MAP_SHARED,g_iFdLcd,0);
	close(g_iFdLcd);

	stride = cairo_format_stride_for_width(CAIRO_FORMAT_RGB24,LCD_WIDTH);

	unlink(KEY_FILE_PATH);
	if (mkfifo(KEY_FILE_PATH, O_CREAT|O_EXCL|0777)<0){
		perror("mkfifo:");
		return -1;
	}

	fd_fifo = open(KEY_FILE_PATH, O_RDWR|O_NONBLOCK);
	if (fd_fifo<0){
		perror("open:");
		return -1;
	}
	//key press event
	g_signal_connect(G_OBJECT(window), "key-press-event", 
			G_CALLBACK(on_key_callback), NULL);

	g_signal_connect(G_OBJECT(window), "key-release-event",
			G_CALLBACK(on_key_callback), NULL);

	gtk_main ();

	return 0;
}
