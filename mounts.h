#ifndef MOUNTS_H
#define MOUNTS_H

#include <string>
#include <vector>
#include <map>

class Mounts
{
public:
   /// absoluteBasePath wird im root mountPath "/" gemounted
   Mounts(std::string const & absoluteBasePath);

   /// liefert den absoluten Pfad für jeden beliebigen mountPath
   std::string absolutePath(std::string const & mountPath) const;

   /// liefert alle gemounteten Pfade zu einem absoluten Pfad
   std::vector<std::string> allMountPathes(std::string const & absolutePath) const;

   /// sucht alle Mountpunkte als pair<mountPath, absolutePath> unterhalb einem mountPath
   std::vector<std::pair<std::string, std::string>> getMountPointsBelow(std::string const & mountPath) const;

   /// fügt einen neuen Mauntpunkt hinzu
   void mount(std::string const & mountPath, std::string const & absolutePath);

   /// löscht einen Mountpunkt an dem absoluten Pfad
   void unmountAbsolute(std::string const & absolutePath);

   /// löscht alle Mountpunkte im mountPath
   void unmountBelow(std::string const & mountPath);

private:
   struct MountPoint;
   using MountPathToMointPoint = std::map<std::string, MountPoint>;
   using AbsolutePath = std::string;
   using MountPath = std::string;
   struct MountPoint
   {
       AbsolutePath absolutePath;
       MountPathToMointPoint::iterator up;
   };

   MountPathToMointPoint m_MountPathToMountpoint;
};

#endif // MOUNTS_H
