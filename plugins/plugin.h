#ifndef PLUGIN_H
#define PLUGIN_H
class Plugin;

//stl container & utillities
#include <map>
#include <string>
#include <set>

//boost smart pointers
//#include <boost/shared_ptr.hpp>

//using namespace boost;



/*! @brief put this behind your sub class definition
 *
 * When ever you create a subclass, this macro should be included in the
 * .cpp file. Suppose you create a module UltraVCO. Your header file should
 * look as usual. your cpp file should look something like this:
 *
 * <code>
 * #include<something.h>
 * #include<somethingelse.h>
 *
 * (...)
 *
 * #include<ultravco.h>
 *
 * (...your method implementations...)
 *
 * PLUGIN_AUTOLOAD(UltraVCO)
 *
 * </code>
 *
 * This should register the plugin UltraVCO as soon as you <code>dlopen</code>
 * the corresponding library file. So all you have to do now is put the
 * compiled modules into some directory, read it's contents at runtime, open 
 * the libs and your are set. (i think you have to dlopen with RTLD_NOW, not
 * RTLD_LAZY, but i'm not 100% sure about this.
 *
 * @TODO look up the authors name of that article in the linux journal
 * and give him kudos.
 */
#define PLUGIN_AUTOLOAD(_m) \
  extern "C" {\
    Plugin * _m##_create(){\
      _m * mod = new _m();\
      mod->setKey(#_m);\
      return mod;\
    }\
\
    class proxy {\
      public:\
      proxy(){\
        Plugin::create[#_m] = _m##_create;\
        Plugin::infos[#_m] = _m::info();\
      }\
    };\
\
    proxy p;\
  }\

/*! fuction Type for creator callbacks that are registered on dlopen.
 *  they should invoke the plugins default and return a (smart) pointer
 *  to the instance created with new.
 *  @note this callbacks are created by the PLUGIN_AUTOLOAD macro above.
 */
//typedef shared_ptrPlugin> plugin_ptr;
typedef Plugin * plugin_ptr;
typedef plugin_ptr maker_t(); 
//typedef void destroyer_t(Plugin *);

/*! A PlugingInfo record is associated with a plugin KEY,
 * which is exactly the key in the create and infos map.
 * this key should be a unique string. Usualy it is just the class name,
 * but the class name does not necessarily have to be unique.
 * The record is not associated with any particular instance.
 * Such information may be provided by subclasses, but we
 * don't care about that at this point.
 */
struct PluginInfo{
  /*! this _MUST_ be the classname of the instances created from this
   *  plugin. Nothing else. No creative energy requirerd at this point.
   */ 
  string className;
  
  string version;
  /*! the synopsis should be the class name of the nearest anchestor
   *  that is known to the framework, i.e. an abstract class like
   *  Module or Parameter etc., whose interface is explicitly
   *  known (as a header file) before and without the need of loading
   *  any plugin. This entry is supposed to give the framework a
   *  vague idea what this plugin is to be used for.
   */
  string synopsis; 
  
  /*! This should be a short keyword indicating the plugin system this 
   *  component expects to work in. You can think of it as a namespace
   *  for interfaces. This will almost certainly be nothing else but
   *  "AMS" + maybe a version number. (unless someone else thinks 
   *  this plugin framework is cool :-) )
   */
  string world;
  
  /*! well... guess what.
   */
  string author;
  
  /*! the licens under which this plugin class is distributed.
   *
   * please don't include the whole text here, but only the
   * keyword (e.g. "GPL", "LGPL", "BSD") see COPYING and README for details.
   * When the user requests the licens agreement for a certain plugin,
   * the framework will look up the key and present the whole text to the 
   * user.(This feature is not yet implemented. i plan to put all known 
   * licens agreements as txt files into a subdir licens, their filename 
   * will be the corresponding licens key.)
   *
   * IMPORTANT NOTE:
   * If this field is empty, or the included key is unknown, the
   * plugin will not be loaded. This is not ment to terrorise you, but
   * to ensure that i am not doing anything illegal without even 
   * noticing. If you have any licens in mind, that isn't known to
   * ams yet, please contact me (AFBLukas_AT_gmx_DOT_de). If it doesn't
   * collide with anything else in ams, i will propably agree to include it.
   * 
   * please read the README.Plugins file for more details.
   */
  string licens;
  
  /*! this field is free form :-)
   */ 
  string description;

  /*! this should be a list of virtual keys this plugin provides.
   *  It should not contain concrete plugin keys.
   *  Think of this as of tagging interfaces in java. it tells any other
   *  part of the framework (including other plugins in particular)
   *  which functionality is implemented by this plugin.
   *  i am not completely sure how to make this explicit right now.
   *  i suggest that for now we use the class names of pure virtual classes,
   *  that are known to ams, so that the implied functionalities can be 
   *  accessed by casting to that classes.
   *  Another way to think of the provides field is to see it in analogy
   *  to the dependency system used e.g. in debian or redhat packages.
   */
  set<string> provides;

  /*! this should be a list of virtual or concrete keys, that express 
   *  requirements of this plugin to become instantiated.
   *  The idea is again similar to that of debian or redhat dependencies.
   *
   *  I.e. if the plugin needs a particular functionality implemented
   *  by some plugin it needs to instantiate, it will either state the
   *  concrete plugin key of that plugin, or a virtual key of which the plugin
   *  knows, that it will do the job within this list.
   *
   *  Indirect requirements, i.e. other interfaces needed by an interface 
   *  required by this plugin should not be listed here, unless you require 
   *  a particular version of a sub-interface.
   *
   *  In case of concrete requirements, it is also valid to request certain
   *  versions of a plugin. so e.g. if the plugin needs version >=0.42 of 
   *  another plugin with key "zaphod", it will put an entry "zaphod>=0.42" 
   *  into this list.
   *
   */
  set<string> requires;
};

/*! @brief A minimalistic plugin component, linkable at runtime 
 *  
 *  Anything that is supposed to be linked into ams at runtime
 *  has to inherit this class as a minimum requirement.
 *
 *  Apart of that the following conditions must be met:
 *
 *   - a plugin has to be default constructable
 *   - the destructor method has to be virtual.
 *   - a plugin must overwrite the info() method.
 *     (This will change as soon as i found a better way of
 *      registering the info)
 *   - within the implementation of each plugin, the
 *     above macro PLUGIN_AUTOLOAD should be included
 *
 *  Whatever else you decide to do with your plugin is not
 *  specified at this point. 
 */
class Plugin{
 
  private:

  /*! this is an integer key, that is guarantied to be unique
   *  for each instance of any plugin (Unique Plugin Instance Id)
   */
  int mUPIId;
  /*! this holds the string key which was used when the plugin class was registered.
   *  this should be usefull for some "rtti-lite" issues :-)
   *  
   *  It could be anything, but it has to be unique.
   */
  string mPluginKey;
  
  /*! this is incremented each time a plugin instance is created.
   *  Don't rely on it, it might be replaced by a more
   *  sophisticated id creation mechanism.
   */
  static int mLastUPIId;


  public:
  static map<string,maker_t *> create;
  
  /*!
   * this contains infos about all registered plugins
   */
  static map<string,PluginInfo> infos;

  /*!
   * @returns the plugin instance unique identifier
   */
  int upiid();

  /*! returns the plugin key for this instance.
   *
   *  The returned key is unique for each plugin _class_
   *  in contrary to the upiid which is unique for each
   *  _instance_
   *
   *  Don't confuse this two terms.
   */
  const string & pluginKey();

  /*! Called by the maker function, to initialize the pluginkey.
   *  Will only have effect if the key is empty at that point.
   *  This method neither should nore does it need to be
   *  invoked manualy.
   *
   *  @TODO this is UGLY, please, @FIXME Find a better way,
   *  for setting pluginkeys. pluginkeys should actualy be static!
   */
  void setKey(const string & key);

  /*! The constructor.
   * does nothing but create an empty plugin instance
   */
  Plugin();

  
  /*!
   * For derived plugins to be decomposed correctly, it is
   * IMPORTANT to make the destructor(s) virtual.
   */
  virtual ~Plugin();
	  
  /*!
   * this method is called by the creator proxy upon
   * autoload of the class. The information it returns
   * is registered in the infos map. Every subclass
   * should overwrite it.
   *
   * this should actualy be pure virtual. But on the other hand
   * it has be static, since we want it to be available _before_
   * instanciation of the plugin.
   * Ouch. What a mess.
   */
  static PluginInfo  info();

  /*! stay informed about the plugins state by registering as a
   *  PluginObserver.
   *  
   *  Right now, this only contains notification about the plugin 
   *  instance death. The context string is freeform. It will be
   *  given back to you during notification, so you can find out
   *  in which context you registered with this plugin.
   */
  //virtual void addPluginListener(PluginListener * listener, const string context);

  /*! unregister your listener 
   */
 //virtual void removePluginListener(PluginListener * listener);
};

#endif
