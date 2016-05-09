Aufgabe
=======

Schreibe eine Klasse die das Mounten analog zu Linux Dateisystemen implementiert.

Beispiel Mountpunkte
--------------------

* `<mountPath>` => `<absolutePath>`
* `"/"` => `"C:\\Users\\admin\\documents"`
* `"/work"` => `"W:\\Dokumente"`
* `"/nas"` => `"N:\\documents"`
* `"/work/kunde1"` => `"W:\\kunde1"`
* `"/work/kunde2"` => `"W:\\kunde2"`
* `"/work/kunde1/shared"` => `"W:\\shared"`
* `"/work/kunde2/shared"` => `"W:\\shared"`

Interface
---------

```cpp
class Mounts
{
public:
   /// absoluteBasePath wird im root mountPath "/" gemounted
   Mounts(string absoluteBasePath);

   /// liefert den absoluten Pfad für jeden beliebigen mountPath
   string absolutePath(string mountPath) const;

   /// liefert alle gemounten Pfade zu einem absoluten Pfad
   vector<string> allMountPathes(string absolutePath) const;

   /// sucht alle Mountpunkte als pair<mountPath, absolutePath> unterhalb einem mountPath
   vector<pair<string, string>> getMountPointsBelow(string mountPath) const;

   /// fügt einen neuen Mauntpunkt hinzu
   void mount(string mountPath, string absolutePath);

   /// löscht einen Mountpunkt an dem absoluten Pfad
   void unmountAbsolute(string absolutePath);

   /// löscht alle Mountpunkte im mountPath
   void unmountBelow(string mountPath);
};
```

Beispiel Ergebnisse
===================

```cpp
$> absolutePath("/work/kunde1/Vertrag.doc");
=> "W:\\kunde1\\Vertrag.doc"
```

```cpp
$> allMountPathes("W:\\shared\\Styleguide.md");
=> ["/work/kunde1/shared/Styleguide.md", "/work/kunde2/shared/Styleguide.md"]
```

```cpp
$> getMountPointsBelow("/work/kunde1");
=> [pair("/work/kunde1", "W:\\kunde1"), pair("/work/kunde1/shared", "W:\\shared")]
```
